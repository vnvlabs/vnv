#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <limits>

#include "base/Communication.h"
#include "base/DistUtils.h"
#include "base/Runtime.h"
#include "base/exceptions.h"
#include "base/Utilities.h"
#include "c-interfaces/Logging.h"
#include "plugins/engines/jsonstreaming/JsonStreamingEngineManager.h"

using nlohmann::json;






namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

namespace JSD {

std::string BEGIN(long id) { return "\n" + std::to_string(id) + ":"; }
std::string END(long id) { return "\n"; }

}

class JsonFileStream : public JsonStream {
  std::string filestub;
  std::map<long, std::ofstream> streams;

  std::string getFileName(long id, bool makedir = true) {
    std::vector<std::string> fname = {std::to_string(id)};
    return getFileName_(filestub, {std::to_string(id) + JSD::extension}, true);
  }

  std::string getFileName_(std::string root, std::vector<std::string> fname,
                           bool mkdir) {
    fname.insert(fname.begin(), root);
    std::string filename = fname.back();
    fname.pop_back();
    std::string fullname = VnV::DistUtils::join(fname, 0777, mkdir);
    return fullname + filename;
  }

  bool remove = false;

 public:
  virtual void initialize(std::string filestub) override {
    this->filestub = filestub;
  }


  virtual void finalize() override {
    // Close all the streams
    json meta = json::array();
    for (auto& it : streams) {
      meta.push_back(it.first);
      it.second.close();
    }

    // Write a little metadata file so we can get all the streams later
    std::ofstream off(getFileName_(filestub, {".meta"}, false));
    off << meta.dump();
    off.close();
  }

  virtual void newComm(long id, json obj) override {
    if (streams.find(id) == streams.end()) {
      std::ofstream off(getFileName(id));
      streams.insert(std::make_pair(id, std::move(off)));
      write(id, obj, -1);
    }
  };

  virtual void write(long id, json obj, long jid) override {
    auto it = streams.find(id);
    if (it != streams.end()) {
      it->second << JSD::BEGIN(jid) << obj.dump() << JSD::END(jid);
    }
  };
};

#define LTypes X(double) X(long long) X(bool) X(std::string) X(json)
#define X(typea)                                                         \
  void JsonStreamingEngineManager::Put(                                  \
      std::string variableName, const typea& value, const MetaData& m) { \
    if (comm->Rank() == getRoot()) {                                     \
      json j;                                                            \
      j[JSD::name] = variableName;                                       \
      j[JSD::dtype] = #typea;                                            \
      j[JSD::value] = value;                                             \
      j[JSD::shape] = {};                                                \
      j[JSD::node] = JSN::shape;                                         \
      j[JSD::meta] = m;                                                  \
      write(j);                                                          \
    }                                                                    \
  }  // namespace Engines
LTypes
#undef X
#undef LTypes

    void
    JsonStreamingEngineManager::Put(std::string variableName,
                                    IDataType_ptr data, const MetaData& m) {

  if (comm->Rank() == getRoot()) {
    json j;
    j[JSD::node] = JSN::dataTypeStarted;
    j[JSD::name] = variableName;
    j[JSD::meta] = m;
    j[JSD::dtype] = data->getKey();
    write(j);
  }

  data->Put(this);

  if (comm->Rank() == getRoot()) {
    json j;
    j[JSD::node] = JSN::dataTypeEnded;
    write(j);
  }
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
    j[JSD::name] = variableName;
    j[JSD::node] = JSN::shape;
    j[JSD::shape] = gsizes;
    j[JSD::dtype] = "ga";
    j[JSD::key] = dtype;
    j[JSD::meta] = m;
    j[JSD::children] = json::array();

    std::string vName = "";
    void* outdata;

    for (int i = 0; i < gather.size(); i++) {
      IDataType_ptr d = gather[i];
      json cj;
      std::map<std::string, PutData> types = d->getLocalPutData();
      for (auto& it : types) {
        json childJson;

        PutData& p = it.second;
        outdata = d->getPutData(it.first);
        childJson[JSD::name] = p.name;
        childJson[JSD::shape] = p.shape;
        int count = std::accumulate(p.shape.begin(), p.shape.end(), 1,
                                    std::multiplies<>());

        switch (p.datatype) {
        case SupportedDataType::DOUBLE: {
          double* dd = (double*)outdata;
          std::vector<double> da(dd, dd + count);
          childJson[JSD::value] = da;
          childJson[JSD::dtype] = "double";

          break;
        }

        case SupportedDataType::LONG: {
          long* dd = (long*)outdata;
          std::vector<long> da(dd, dd + count);
          childJson[JSD::value] = da;
          childJson[JSD::dtype] = "long";

          break;
        }

        case SupportedDataType::STRING: {
          std::string* dd = (std::string*)outdata;
          std::vector<std::string> da(dd, dd + count);
          childJson[JSD::value] = da;
          childJson[JSD::dtype] = "std::string";

          break;
        }

        case SupportedDataType::JSON: {
          json* dd = (json*)outdata;
          std::vector<json> da(dd, dd + count);
          childJson[JSD::value] = da;
          childJson[JSD::dtype] = "json";
          break;
        }
        }
        cj[p.name] = childJson;
      }
      j[JSD::children].push_back(cj);
    }
    write(j);
  }
}

void JsonStreamingEngineManager::Log(ICommunicator_ptr logcomm,
                                     const char* package, int stage,
                                     std::string level, std::string message) {
  
  //Save the current communicator
  ICommunicator_ptr commsave = this->comm;

  //Set the logcomm as the current communication
  setComm(logcomm,true);

  //Write the log
  if (comm->Rank() == getRoot()) {
    json log = json::object();
    log[JSD::package] = package;
    log[JSD::stage] = stage;
    log[JSD::level] = level;
    log[JSD::message] = message;
    log[JSD::node] = JSN::log;
    log[JSD::name] = std::to_string(JsonStreamingEngineManager::id++);
    log[JSD::comm] = comm->uniqueId();
    log[JSD::time] = comm->time();

    write(log);
  }

  // Set the old comm 
  if (commsave != nullptr) {
    setComm(commsave,false);
  }
}

nlohmann::json JsonStreamingEngineManager::getConfigurationSchema() {
  return JSD::__json_engine_schema__;
}

void JsonStreamingEngineManager::finalize(ICommunicator_ptr worldComm) {
  stream->finalize();
}

void JsonStreamingEngineManager::setFromJson(ICommunicator_ptr comm,
                                             nlohmann::json& config) {
  if (config.contains(JSD::outputFile)) {
    this->outputFile = config[JSD::outputFile].get<std::string>();
  }
  stream->initialize(this->outputFile);
  setComm(comm,false);

  if ( comm->Rank() == getRoot() ) {
      
      // This is the first one so we send over the info. . 
      json nJson;
      nJson = json::object();
      nJson["title"] = "VnV Simulation Report";
      nJson["date"] =
              std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();
      nJson[JSD::name] = "MainInfo";
      nJson[JSD::node] = JSN::info;
      nJson["worldSize"] = comm->Size();
      nJson[JSD::spec] = RunTime::instance().getFullJson();
      stream->write(id,nJson, -2);
  }    

}

void JsonStreamingEngineManager::injectionPointEndedCallBack(
    std::string id, InjectionPointType type, std::string stageVal) {
  
  syncId();
  if (comm->Rank() == getRoot()) {
      json j = json::object();
      j[JSD::time] = comm->time();
      j[JSD::node] = (type == InjectionPointType::End || type == InjectionPointType::Single) ? JSN::injectionPointEnded : JSN::injectionPointIterEnded;
      write(j);
  }
   
}

void JsonStreamingEngineManager::injectionPointStartedCallBack(
    ICommunicator_ptr comm, std::string packageName, std::string id,
    InjectionPointType type, std::string stageVal) {
  
  setComm(comm,true);
  
  if (comm->Rank() == getRoot()) {
    json j;
    j[JSD::name] = id;
    j[JSD::package] = packageName;
    j[JSD::comm] = comm->uniqueId();
    j[JSD::time] = comm->time();

    if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
      j[JSD::node] = JSN::injectionPointStarted;
    } else {
      j[JSD::stageId] = stageVal;
      j[JSD::node] = JSN::injectionPointIterStarted;
    }
    write(j);
  }
}

void JsonStreamingEngineManager::testStartedCallBack(std::string packageName,
                                                     std::string testName,
                                                     bool internal, long uuid) {
  
  
  syncId();
  

  if (comm->Rank() == getRoot()) {
    json j;
    j[JSD::name] = testName;
    j[JSD::package] = packageName;
    j[JSD::internal] = internal;
    j[JSD::node] = JSN::testStarted;
    j[JSD:: testuid] = uuid;
    write(j);
  }
}

void JsonStreamingEngineManager::testFinishedCallBack(bool result_) {
  if (comm->Rank() == getRoot()) {
    json j = json::object();
    j[JSD::node] = JSN::testFinished;
    j[JSD::result] = result_;

    write(j);
  }
}

void JsonStreamingEngineManager::unitTestStartedCallBack(
    ICommunicator_ptr comm, std::string packageName, std::string unitTestName) {
  json j;

  setComm(comm, true);

  if (comm->Rank() != getRoot()) {
    j[JSD::node] = JSN::unitTestStarted;
    j[JSD::name] = unitTestName;
    j[JSD::package] = packageName;
    write(j);
  }
}

void JsonStreamingEngineManager::unitTestFinishedCallBack(IUnitTest* tester) {
  if (comm->Rank() == getRoot()) {
    json j = json::object();
    j[JSD::node] = JSN::unitTestFinished;
    json c = json::array();
    for (auto it : tester->getResults()) {
      json kk = json::object();
      kk[JSD::name] = std::get<0>(it);
      kk[JSD::description] = std::get<1>(it);
      kk[JSD::result] = std::get<2>(it);
      c.push_back(kk);
    }
    j[JSD::results] = c;
    write(j);
  }
}

std::shared_ptr<Nodes::IRootNode> JsonStreamingEngineManager::readFromFile( std::string file, long& idCounter) {
   return parseJsonFileStream(idCounter,file);
}

std::string JsonStreamingEngineManager::print() {
  return "VnV Json Streaming Engine Manager";
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV

INJECTION_ENGINE(VNVPACKAGENAME, jsonfilestream) {
  return new VnV::VNVPACKAGENAME::Engines::JsonStreamingEngineManager(
      std::make_shared<VnV::VNVPACKAGENAME::Engines::JsonFileStream>());
}