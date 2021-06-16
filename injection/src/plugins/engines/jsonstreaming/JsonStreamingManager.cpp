﻿#include "plugins/engines/jsonstreaming/JsonStreamingEngineManager.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>

#include "base/Communication.h"
#include "base/Runtime.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "plugins/engines/json/JsonOutputReader.h"

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

};
/**

  This is the documentation.

**/
INJECTION_ENGINE(VNVPACKAGENAME, jsonfilestream) {
  return new VnV::VNVPACKAGENAME::Engines::JsonStreamingEngineManager(std::make_shared<JsonFileStream>());
}

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

#define LTypes X(double) X(long long) X(bool) X(std::string) X(json)
#define X(type)                                                            \
  void JsonStreamingEngineManager::Put(std::string variableName, const type& value, \
                              const MetaData& m) {                         \
    if (comm->Rank() != getRoot()) return;                                 \
                                                                           \
    json j;                                                                \
    j["id"] = JsonStreamingEngineManager::id++;                                     \
    j["name"] = variableName;                                              \
    j["type"] = #type;                                                     \
    j["value"] = value;                                                    \
    j["node"] = "Data";                                                    \
    j["meta"] = m;                                                         \
    write(j);                                                              \
  }
LTypes
#undef X
#undef LTypes

void JsonStreamingEngineManager::Put(std::string variableName, IDataType_ptr data, const MetaData& m) {

  if (comm->Rank() == getRoot()) {
    json j;
    j["id"] = JsonStreamingEngineManager::id++;
    j["node"] = "DataType";
    j["name"] = variableName;
    j["meta"] = m;
    j["dtype"] = data->getKey();
    j["children"] = json::array();
    j["results"] = json::array();
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

  //Gather all on the root processor
  IDataType_vec gather =
      d.GatherV(data, dtype, getRoot(), gsizes, sizes, offset, false);

  if (gather.size() > 0 && comm->Rank() == getRoot()) {
    json j;
    j["id"] = JsonStreamingEngineManager::id++;
    j["name"] = variableName;
    j["node"] = "Data";
    j["type"] = "shape";
    j["shape"] = gsizes;
    j["meta"] = m;
    j["children"] = json::array();
    push(j, json::json_pointer("/children"));

    std::string vName = "";
    void* outdata;

    for (int i = 0; i < gather.size(); i++) {
      IDataType_ptr d = gather[i];
      json cj;
      int iter = 0;
      std::vector<PutData> types = d->getLocalPutData();
      for (int j = 0; j < types.size(); j++) {
        PutData& p = types[j];
        outdata = d->getPutData(j);
        cj["name"] = p.name;

        switch (p.datatype) {

        case SupportedDataType::DOUBLE: {
          double* dd = (double*)outdata;
          if (p.count > 1) {
            std::vector<double> da(dd, dd + p.count);
            cj["value"] = da;
          } else {
            cj["value"] = *dd;
          }
          break;
        }

        case SupportedDataType::LONG: {

          long* dd = (long*)outdata;
          if (p.count > 1) {
            std::vector<long> da(dd, dd + p.count);
            cj["value"] = da;
          } else {
            cj["value"] = *dd;
          }
          break;
        }

        case SupportedDataType::STRING: {

          std::string* dd = (std::string*)outdata;
          if (p.count > 1) {
            std::vector<std::string> da(dd, dd + p.count);
            cj["value"] = da;
          } else {
            cj["value"] = *dd;
          }
          break;
        }

        case SupportedDataType::JSON: {

          json* dd = (json*)outdata;
          if (p.count > 1) {
            std::vector<json> da(dd, dd + p.count);
            cj["value"] = da;
          } else {
            cj["value"] = *dd;
          }
          break;
        }
        }

        append(cj);
      }
    }
    pop(2);
  }
}

void JsonStreamingEngineManager::Log(ICommunicator_ptr logcomm, const char* package,
                            int stage, std::string level, std::string message) {
  // Logs before comm is set.

  auto id = logcomm->uniqueId();
  commMapper.logComm(comm);
  JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
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
    append(log);
  }

  if (commsave != nullptr) {
    setComm(commsave);
  }
}

nlohmann::json JsonStreamingEngineManager::getConfigurationSchema() {
  return __json_engine_schema__;
}

namespace {

template <class I1, class I2>
bool have_common_element(I1 first1, I1 last1, I2 first2, I2 last2) {
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2)
      ++first1;
    else if (*first2 < *first1)
      ++first2;
    else
      return true;
  }
  return false;
}

enum class DataRelative {
  PARENT,
  CHILD,
  OLDERSIBLING,
  YOUNGERSIBLING,
  STRANGER
};

class IData {
 public:
  bool isRoot = false;
  json main;
  json::json_pointer ptr;
  long idstart = -1;
  long idstop = -1;
  long comm = -1;
  std::map<long, std::set<std::shared_ptr<IData>>> children;
  std::set<long> chain;
  CommWrap_ptr commWrap;

  IData(json& m, std::map<long, std::shared_ptr<CommWrap>>& comms,
        bool isR = false)
      : isRoot(isR), main(m) {

    if (!isRoot) {
      std::cout << "Main" << main.dump(3) << std::endl;
      idstart = main["id"].get<long>();
      idstop = main["endid"].get<long>();
      comm = main["comm"].get<long>();
      chain = comms.find(comm)->second->getCommChain();
    }

    if (main.contains("children")) {
      json& cc = main["children"];
      for (auto c = cc.begin(); c != cc.end();) {
        if (c.value()["node"] == "InjectionPoint") {
          addChildInternal(std::make_shared<IData>(c.value(), comms));
          c = cc.erase(c);
        } else {
          ++c;
        }
      }
    } else {
      main["children"] = json::array();
    }
  }

  json write() {
    // Add back all the children to the main array.
    nlohmann::json& cc = main["children"];
    for (auto it : children) {
      for (auto itt : it.second) {
        cc.push_back(itt->write());
      }
    }
    return main;
  }

  void addChildInternal(std::shared_ptr<IData> data) {
    auto it = children.find(data->idstart);
    if (it == children.end()) {
      children[data->idstart] = {data};
    } else {
      children[data->idstart].insert(data);
    }
  }

  DataRelative getRelation(std::shared_ptr<IData> data) {
    bool chainContains = chain.find(data->comm) != chain.end();
    if (!isRoot && !chainContains) {
      return DataRelative::STRANGER;
    } else if (!isRoot && data->idstop < idstart)
      return DataRelative::OLDERSIBLING;
    else if (!isRoot && data->idstart > idstop)
      return DataRelative::YOUNGERSIBLING;
    else if (!isRoot && data->idstart < idstart && data->idstop > idstop) {
      // data->getRelation(this);// Add myself as a child to data.
      return DataRelative::PARENT;
    } else if (isRoot || (data->idstart > idstart && data->idstop < idstop)) {
      bool isNewChild = true;
      for (auto childStartId = children.begin();
           childStartId != children.end();) {
        for (auto child = childStartId->second.begin();
             child != childStartId->second.end();) {
          auto r = (*child)->getRelation(data);
          if (r == DataRelative::CHILD) {
            return DataRelative::CHILD;  // child so its handled --> Return .
          } else if (r == DataRelative::PARENT) {
            // The get realation call has already added child as a child of
            // data.
            data->getRelation(*child);
            child = childStartId->second.erase(child);  // erase this as child.
            isNewChild = true;
            // Can't return just yet, need to check other siblings are not
            // children of it.
          } else {
            child++;  // Its a sibling, so just continue and add at bottom.
          }
        }
        if (childStartId->second.size() == 0) {
          childStartId = children.erase(childStartId);
        } else {
          childStartId++;
        }
      }
      // If we reach here, then its my child.
      addChildInternal(data);
      return DataRelative::CHILD;
    }
    return DataRelative::STRANGER;
  }
};

void join(std::shared_ptr<IData>& datastruct, long commId,
          std::map<long, CommWrap_ptr> comms, std::string outfile,
          std::set<long>& done) {
  // Don't add comms twice.
  auto comm = comms.find(commId)->second;
  if (done.find(commId) != done.end())
    return;
  else
    done.insert(commId);

  // Parse the file into a json object.
  std::string s = outfile + "_" + std::to_string(comm->id);
  std::ifstream f(s);
  json thisJson = json::parse(f);
  f.close();

  if (datastruct == nullptr) {
    datastruct.reset(new IData(thisJson, comms, true));
  } else {
    // Get a root structure going.
    for (auto it : thisJson["children"].items()) {
      if (it.value()["node"] == "InjectionPoint") {
        auto d = std::make_shared<IData>(it.value(), comms);
        datastruct->getRelation(d);
      } else {
        // Just add it to the main structs children.
        datastruct->main["children"].push_back(it.value());
      }
    }
  }
  //
}

void join(std::string outputfile, std::set<CommWrap_ptr>& comms,
          int worldSize) {
  // The goal here it to join all the output files into a single file
  // We should get a single output file that contains all injection points.
  if (comms.size() == 0) return;
  if (comms.size() > 1) {
    throw VnVExceptionBase("To many root communicators");
  }

  json jcomm = json::object();
  std::set<long> done1;
  for (auto it : comms) {
    it->toJson1(jcomm, done1);
  }
  json comMap = json::object();
  comMap["worldSize"] = worldSize;
  comMap["map"] = jcomm;

  std::map<long, CommWrap_ptr> commsMap = CommMapper::convertToMap(comms);
  std::shared_ptr<IData> dstruct = nullptr;
  std::set<long> done;
  for (auto it : commsMap) {
    join(dstruct, it.second->id, commsMap, outputfile, done);
  }
  json joinedJson = dstruct->write();

  joinedJson["commMap"] = comMap;
  joinedJson["spec"] = RunTime::instance().getFullJson();

  std::ofstream f(outputfile);
  f << joinedJson.dump(4);
  f.close();
}

}  // namespace

void JsonStreamingEngineManager::finalize(ICommunicator_ptr worldComm) {
  // finalize gets called on the world comm. For now, lets have every one dump
  // the comm info they own to file.

  // TODO -- Could cut this down to just what we need. Right now, we export
  // everthing.
  auto p = commMapper.gatherCommInformation(worldComm);
  if (!outputFile.empty()) {
    for (auto it : mainJson.items()) {
      std::ofstream f;
      std::ostringstream oss;
      oss << outputFile << "_" << it.key();
      f.open(oss.str());
      f << it.value().dump(3);
      f.close();
    }

    // Finish up the FileIO
    worldComm->Barrier();
    if (worldComm->Rank() == getRoot()) {
      join(outputFile, p, worldComm->Size());
    }

  } else {
    std::cout << Dump(3);
  }
}

void JsonStreamingEngineManager::setFromJson(ICommunicator_ptr comm, nlohmann::json& config) {
  if (config.contains("inMemory")) {
    this->inMemory = config["inMemory"].get<bool>();
  }
  if (config.contains("outputFile")) {
    this->outputFile = config["outputFile"].get<std::string>();
  }
  setComm(comm);
}

void JsonStreamingEngineManager::injectionPointEndedCallBack(std::string id,
                                                    InjectionPointType type,
                                                    std::string stageVal) {
  JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);

  if (type == InjectionPointType::End || type == InjectionPointType::Single) {
    pop(3);
    JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
    if (comm->Rank() != getRoot()) return;
    add("endid", JsonStreamingEngineManager::id++);
    pop(1);
  } else {
    // This was an iter so pop back one and set the endid, then pop another to
    // exit the stage
    if (comm->Rank() != getRoot()) return;
    pop(2);
  }
}

void JsonStreamingEngineManager::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                      std::string packageName,
                                                      std::string id,
                                                      InjectionPointType type,
                                                      std::string stageVal) {
  setComm(comm);
  json ip;
  json stage;

  stage["name"] = id;
  stage["package"] = packageName;
  stage["stageId"] = stageVal;
  stage["children"] = json::array();
  stage["node"] = "InjectionPointStage";

  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
    if (comm->Rank() != getRoot()) return;

    json j;
    j["node"] = "InjectionPoint";
    j["name"] = id;
    j["id"] = JsonStreamingEngineManager::id++;
    j["package"] = packageName;
    j["children"] = json::array();
    j["comm"] = comm->uniqueId();
    stage["id"] = JsonStreamingEngineManager::id++;
    j["children"].push_back(stage);
    push(j, json::json_pointer("/children/0/children"));
    // Now we are pointing to the child node, so all test stuff there.
  } else {
    if (comm->Rank() != getRoot()) return;
    stage["id"] = JsonStreamingEngineManager::id++;
    push(stage, json::json_pointer("/children"));
  }
}

void JsonStreamingEngineManager::testStartedCallBack(std::string packageName,
                                            std::string testName,
                                            bool internal) {
  JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  if (comm->Rank() != getRoot()) return;

  json j;
  j["id"] = JsonStreamingEngineManager::id++;
  j["name"] = testName;
  j["package"] = packageName;
  j["internal"] = internal;
  j["node"] = "Test";
  j["children"] = json::array();
  push(j, json::json_pointer("/children"));
}

void JsonStreamingEngineManager::testFinishedCallBack(bool result_) {
  if (comm->Rank() != getRoot()) return;
  pop(2);
}

void JsonStreamingEngineManager::unitTestStartedCallBack(ICommunicator_ptr comm,
                                                std::string packageName,
                                                std::string unitTestName) {
  json j;
  setComm(comm);
  JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  if (comm->Rank() != getRoot()) return;

  j["id"] = JsonStreamingEngineManager::id++;
  j["node"] = "UnitTest";
  j["name"] = unitTestName;
  j["package"] = packageName;
  j["children"] = json::array();
  j["results"] = json::array();
  j["comm"] = comm->uniqueId();
  push(j, json::json_pointer("/children"));
}


void JsonStreamingEngineManager::unitTestFinishedCallBack(IUnitTest* tester) {
  if (comm->Rank() != getRoot()) return;  // pop the children node
  pop(1);

  // push to the results node
  append(json::json_pointer("/results"));
  for (auto it : tester->getResults()) {
    Put(std::get<0>(it), std::get<2>(it), MetaData());
  }

  // pop the results and the unit-test itself.
  pop(2);
}

Nodes::IRootNode* JsonStreamingEngineManager::readFromFile(std::string file,
                                                  long& idCounter) {
  return VnV::VNVPACKAGENAME::Engines::JsonReader::parse(file, idCounter);
}

std::string JsonStreamingEngineManager::print() { return "VnV Json Engine Manager"; }

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
