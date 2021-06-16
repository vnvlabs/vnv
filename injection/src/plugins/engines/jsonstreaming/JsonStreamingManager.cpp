#include <chrono>
#include <fstream>
#include <iostream>
#include <set>

#include "base/Communication.h"
#include "base/Runtime.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "plugins/engines/jsonstreaming/JsonStreamingEngineManager.h"

using nlohmann::json;

static json __json_engine_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv-debug-schema.json",
    "title": "Json Engine Input Schema",
    "description": "Schema for the debug engine",
    "type": "object",
    "parameters" : {
       "outputFile" : {"type":"string"},
       "inMemory" : {"type" : "boolean" , "default" : true }
     }
}
)"_json;

class JsonFileStream : public VnV::VNVPACKAGENAME::Engines::JsonStream {

  std::string BEGIN = "\n********BEGIN********\n";
  std::string END = "\n********END********\n";



  std::string filestub;
  std::map<long, std::ofstream> streams;
  std::string getFileName(long id) {
    return filestub + std::to_string(id) + ".jstream";
  }


  public:
  
  virtual void initialize(std::string filestub) override {
    this->filestub = filestub;
  }
  virtual void finalize() override {
     for (auto &it : streams) {
        it.second.close();
     }
  }


  virtual void newComm(long id, json obj) override {
      if (streams.find(id) == streams.end() ) {
          std::ofstream off(getFileName(id));
          streams.insert(std::make_pair(id, std::move(off)));  
          write(id,obj);
      }
  };

  virtual void write(long id, json obj) override {
     auto it = streams.find(id);
     if (it != streams.end() ) {
        it->second << BEGIN << obj.dump() << END;
     }
  } ;

  virtual void parse(std::string filename) override {}//TODO;
  virtual json next() override { return json::object();} //TODO ;

};
/**

  This is the documentation.

**/
INJECTION_ENGINE(VNVPACKAGENAME, jsonfilestream) {
  return new VnV::VNVPACKAGENAME::Engines::JsonStreamingEngineManager(
      std::make_shared<JsonFileStream>());
}

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

#define LTypes X(double) X(long long) X(bool) X(std::string) X(json)
#define X(type)                                                                \
  void JsonStreamingEngineManager::Put(std::string variableName,               \
                                       const type& value, const MetaData& m) { \
    if (comm->Rank() == getRoot()) {                                           \
      json j;                                                                  \
      j["id"] = JsonStreamingEngineManager::id++;                              \
      j["name"] = variableName;                                                \
      j["type"] = #type;                                                       \
      j["value"] = value;                                                      \
      j["shape"] = {};                                                         \
      j["node"] = "Data";                                                      \
      j["meta"] = m;                                                           \
      write(j);                                                                \
    }                                                                          \
  }  // namespace Engines
LTypes
#undef X
#undef LTypes

    void
    JsonStreamingEngineManager::Put(std::string variableName,
                                    IDataType_ptr data, const MetaData& m) {

  if (comm->Rank() == getRoot()) {
    json j;
    j["id"] = JsonStreamingEngineManager::id++;
    j["node"] = "DataType";
    j["name"] = variableName;
    j["meta"] = m;
    j["dtype"] = data->getKey();
    write(j);
  }

  data->Put(this);
}

std::string JsonStreamingEngineManager::getId() {
  return std::to_string(JsonStreamingEngineManager::id++);
}

void JsonStreamingEngineManager::PutGlobalArray(
    long long dtype, std::string variableName, IDataType_vec data,
    std::vector<int> gsizes, std::vector<int> sizes, std::vector<int> offset,
    const MetaData& m) {
  
  VnV::Communication::DataTypeCommunication d(comm);

  // Gather all on the root processor
  IDataType_vec gather =
      d.GatherV(data, dtype, getRoot(), gsizes, sizes, offset, false);

  if (gather.size() > 0 && comm->Rank() == getRoot()) {
    json j;
    j["id"] = JsonStreamingEngineManager::id++;
    j["name"] = variableName;
    j["node"] = "Data";
    j["type"] = "shape";
    j["shape"] = gsizes;
    j["key"] = dtype;
    j["meta"] = m;
    j["children"] = json::array();
    
    std::string vName = "";
    void* outdata;

    for (int i = 0; i < gather.size(); i++) {
      IDataType_ptr d = gather[i];
      json cj;
      std::map<std::string,PutData> types = d->getLocalPutData();
      for (auto &it : types) {
        json childJson;

        PutData& p = it.second;
        outdata = d->getPutData(it.first);
        childJson["name"] = p.name;
        childJson["shape"] = p.shape;
        childJson["type"] = p.datatype;
        int count = std::accumulate(p.shape.begin(), p.shape.end(), 1, std::multiplies<>());

        switch (p.datatype) {

        case SupportedDataType::DOUBLE: {
          double* dd = (double*)outdata;
          std::vector<double> da(dd, dd + count);
          childJson["value"] = da;
          break;
        }

        case SupportedDataType::LONG: {
          long* dd = (long*)outdata;
          std::vector<long> da(dd, dd + count);
          childJson["value"] = da;
          break;
        }

        case SupportedDataType::STRING: {
          std::string* dd = (std::string*)outdata;
          std::vector<std::string> da(dd, dd + count);
          childJson["value"] = da;
          break;
        }

        case SupportedDataType::JSON: {
          json* dd = (json*)outdata;
          std::vector<json> da(dd, dd + count);
          childJson["value"] = da;
         break;
        }
        }
        cj[p.name] = childJson;
      }
      j["children"].push_back(cj);
    }
    write(j);
  }

}

void JsonStreamingEngineManager::Log(ICommunicator_ptr logcomm,
                                     const char* package, int stage,
                                     std::string level, std::string message) {
  // Logs before comm is set.

  auto id = logcomm->uniqueId();
  commMapper.logComm(comm);
  JsonStreamingEngineManager::id =
      commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  ICommunicator_ptr commsave = this->comm;

  setComm(comm);

  if (comm->Rank() == getRoot()) {
    json log = json::object();
    log["package"] = package;
    log["stage"] = stage;
    log["level"] = level;
    log["message"] = message;
    log["node"] = "Log";
    log["id"] = JsonStreamingEngineManager::id;
    log["name"] = std::to_string(JsonStreamingEngineManager::id++);
    log["comm"] = id;
    write(log);
  }

  if (commsave != nullptr) {
    setComm(commsave);
  }
}

nlohmann::json JsonStreamingEngineManager::getConfigurationSchema() {
  return __json_engine_schema__;
}

void JsonStreamingEngineManager::finalize(ICommunicator_ptr worldComm) {
  stream->finalize();
}

void JsonStreamingEngineManager::setFromJson(ICommunicator_ptr comm,
                                             nlohmann::json& config) {
  if (config.contains("outputFile")) {
    this->outputFile = config["outputFile"].get<std::string>();
  }
  stream->initialize(this->outputFile);
  setComm(comm);
}

void JsonStreamingEngineManager::injectionPointEndedCallBack(
    std::string id, InjectionPointType type, std::string stageVal) {
  JsonStreamingEngineManager::id =
      commMapper.getNextId(comm, JsonStreamingEngineManager::id);

  if (type == InjectionPointType::End || type == InjectionPointType::Single) {
    JsonStreamingEngineManager::id =
        commMapper.getNextId(comm, JsonStreamingEngineManager::id);
    if (comm->Rank() == getRoot()) {
      json j = json::object();
      j["endid"] = JsonStreamingEngineManager::id++;
      j["type"] = "InjectionPointEnded";
      write(j);
    }
  } else {
    if (comm->Rank() == getRoot()) {
      json j = json::object();
      j["type"] = "InjectionPointIterEnded";
      write(j);
    }
  }
}

void JsonStreamingEngineManager::injectionPointStartedCallBack(
    ICommunicator_ptr comm, std::string packageName, std::string id,
    InjectionPointType type, std::string stageVal) {
  setComm(comm);
  json ip;
  json stage;

  stage["name"] = id;
  stage["package"] = packageName;
  stage["stageId"] = stageVal;
  stage["node"] = "InjectionPointStage";

  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    JsonStreamingEngineManager::id =
        commMapper.getNextId(comm, JsonStreamingEngineManager::id);
    if (comm->Rank() == getRoot()) {
      json j;
      j["node"] = "InjectionPoint";
      j["name"] = id;
      j["id"] = JsonStreamingEngineManager::id++;
      j["package"] = packageName;
      j["comm"] = comm->uniqueId();
      stage["id"] = JsonStreamingEngineManager::id++;
      write(j);
    }
    // Now we are pointing to the child node, so all test stuff there.
  } else {
    if (comm->Rank() == getRoot()) {
      stage["id"] = JsonStreamingEngineManager::id++;
      write(stage);
    }
  }
}

void JsonStreamingEngineManager::testStartedCallBack(std::string packageName,
                                                     std::string testName,
                                                     bool internal) {
  JsonStreamingEngineManager::id =
      commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  if (comm->Rank() == getRoot()) {
    json j;
    j["id"] = JsonStreamingEngineManager::id++;
    j["name"] = testName;
    j["package"] = packageName;
    j["internal"] = internal;
    j["node"] = "Test";
    write(j);
  }
}

void JsonStreamingEngineManager::testFinishedCallBack(bool result_) {
  if (comm->Rank() == getRoot()) {
    json j = json::object();
    j["type"] = "testFinished";
    j["result"] = result_;
    write(j);
  }
}

void JsonStreamingEngineManager::unitTestStartedCallBack(
    ICommunicator_ptr comm, std::string packageName, std::string unitTestName) {
  json j;
  setComm(comm);
  JsonStreamingEngineManager::id =
      commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  if (comm->Rank() != getRoot()) {
    j["id"] = JsonStreamingEngineManager::id++;
    j["node"] = "UnitTest";
    j["name"] = unitTestName;
    j["package"] = packageName;
    j["comm"] = comm->uniqueId();
    write(j);
  }
}

void JsonStreamingEngineManager::unitTestFinishedCallBack(IUnitTest* tester) {
  if (comm->Rank() == getRoot()) {
    json j = json::object();
    j["type"] = "UnitTestFinished";
    json c = json::array();
    for (auto it : tester->getResults()) {
      json kk = json::object();
      kk["name"] = std::get<0>(it);
      kk["desc"] = std::get<1>(it);
      kk["result"] = std::get<2>(it);
      c.push_back(kk);
    }
    j["results"] = c;
    write(j);
  }
}

namespace JsonStreamingEngineReader {
   Nodes::IRootNode* parse(std::string filename, long idCounter, std::shared_ptr<JsonStream> stream) {
      std::cout << "Json Streaming Engine Reader Is not Implemented Yet.";
      return nullptr;
   }
}

Nodes::IRootNode* JsonStreamingEngineManager::readFromFile(std::string file,
                                                           long& idCounter) {
  return VnV::VNVPACKAGENAME::Engines::JsonStreamingEngineReader::parse(file, idCounter, stream);
}

std::string JsonStreamingEngineManager::print() {
  return "VnV Json Streaming Engine Manager";
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
