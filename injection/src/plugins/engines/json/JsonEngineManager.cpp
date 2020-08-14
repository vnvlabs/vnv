#include "plugins/engines/json/JsonEngineManager.h"
#include <chrono>
#include <fstream>
#include <iostream>

#include "base/Runtime.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "plugins/engines/json/JsonOutputReader.h"
#include "base/Communication.h"

using nlohmann::json;

static json __json_engine_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv-debug-schema.json",
    "title": "Json Engine Input Schema",
    "description": "Schema for the debug engine",
    "type": "object",
    "parameters" : {
       "outputFile" : {"type":"string"}
     }
}
)"_json;

/**

  This is the documentation.

**/
INJECTION_ENGINE(VNVPACKAGENAME, json) {
  return new VnV::VNVPACKAGENAME::Engines::JsonEngineManager();
}

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

#define LTypes X(double) X(int) X(long) X(bool) X(std::string) X(json)
#define X(type)                                                                \
  void JsonEngineManager::Put(std::string variableName,                        \
                              const type& value) {                             \
      if (currComm->Rank() != getRoot(currComm)) return;                           \
      json j;                                                                  \
      j["id"] = getId();                                                       \
      j["name"] = variableName;                                                \
      j["type"] = #type;                                                       \
      j["value"] = value;                                                      \
      j["node"] = "Data";                                                      \
      append(j);                                                               \
  }
LTypes
#undef X
#undef LTypes

std::string JsonEngineManager::getId() {
  return std::to_string(id++);
}

void JsonEngineManager::WriteDataArray(std::string variableName, IDataType_vec &data, std::vector<int> &shape){
  //We only write on the root processor.
  if (currComm->Rank() == getRoot(currComm)) {
     json j;
     j["id"] = getId();
     j["name"] = variableName;
     j["node"] = "Data";
     j["type"] = "shape";
     j["shape"] = shape;
     j["children"] = json::array();

     push(j, json::json_pointer("/children"));
     // The data type should be put on a self communicator because we are on the root now.
     //
     ICommunicator_ptr self = CommunicationStore::instance().selfComm(currComm->getPackage());
     for (int i = 0; i < data.size(); i++ ) {
       dataTypeStartedCallBack(self, std::to_string(i), data[i]->getKey() );
       data[i]->Put(this);
       dataTypeEndedCallBack(self, std::to_string(i));
     }

     pop(2);
   }

}

void JsonEngineManager::append(nlohmann::json& jsonOb) {
  mainJson.at(ptr).push_back(jsonOb);
}

void JsonEngineManager::pop(int num) {
  while (num-- > 0) ptr = ptr.parent_pointer();
}

void JsonEngineManager::push(nlohmann::json& jsonOb) {
  append(jsonOb);
  ptr /= mainJson.at(ptr).size() - 1;
}

void JsonEngineManager::append(json::json_pointer ptr) { this->ptr /= ptr; }

void JsonEngineManager::push(nlohmann::json& jsonOb, json::json_pointer ptr) {
  push(jsonOb);
  this->ptr /= ptr;
}

std::string JsonEngineManager::Dump(int d) { return mainJson.dump(d); }

JsonEngineManager::JsonEngineManager() {
  mainJson = json::object();
  mainJson["info"] = json::object();
  mainJson["info"]["title"] = "VnV Simulation Report";
  mainJson["info"]["date"] =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  mainJson["info"]["id"] = getId();
  mainJson["info"]["name"] = "info";
  mainJson["children"] = json::array();
  mainJson["comm"] = json::object();

  ptr = json::json_pointer("/children");
}




void JsonEngineManager::PutGlobalArray(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, std::vector<int> gsizes, std::vector<int> sizes, std::vector<int> offset, int onlyOne)
{
  VnV::Communication::DataTypeCommunication d(comm);
  if (onlyOne > 0 && onlyOne == comm->Rank() && comm->Rank() == getRoot(comm) ) {
     // We are the root process and the only required information is on this process. So, just call put.
     WriteDataArray(variableName, data, gsizes);
  } else if ( onlyOne > 0 && comm->Rank() == getRoot(comm)) {
     // We are the root -- recv an array of shape gsizes from rank onlyOne
     int s = std::accumulate(gsizes.begin(),gsizes.end(),1, std::multiplies<int>());
     std::pair<IDataType_vec, IStatus_ptr> r = d.Recv(s,dtype, onlyOne, 223);
     WriteDataArray( variableName, r.first, gsizes );
  } else if ( onlyOne > 0 && comm->Rank() == onlyOne) {
     //We are onlyOne, We need to send our information to the root
     d.Send(data, getRoot(comm), 223, true);
  } else {
     //All GatherV
     IDataType_vec rdata = d.GatherV(data, dtype, getRoot(comm), gsizes, sizes, offset, false);
     if (comm->Rank() == getRoot(comm)) {
        WriteDataArray( variableName, rdata, gsizes);
     }

  }
}

void JsonEngineManager::Log(ICommunicator_ptr comm, const char* package, int stage,
                            std::string level, std::string message) {
  auto id = comm->uniqueId();
  if (comm->Rank() == getRoot(comm) ) {
   json log = json::object();
   log["package"] = package;
   log["stage"] = stage;
   log["level"] = level;
   log["message"] = message;
   log["node"] = "Log";
   log["id"] = getId();
   log["name"] = getId();
   log["comm"] = id;
   append(log);
 }

}

nlohmann::json JsonEngineManager::getConfigurationSchema() {
  return __json_engine_schema__;
}

void JsonEngineManager::finalize() {
  // TODO -- Could cut this down to just what we need. Right now, we export
  // everthing.
  mainJson["spec"] = RunTime::instance().getFullJson();

  if (!outputFile.empty()) {
    std::ofstream f;
    f.open(outputFile);
    f << Dump(3);
  } else {
    std::cout << Dump(3);
  }
}

void JsonEngineManager::setFromJson(nlohmann::json& config) {
  if (config.contains("outputFile")) {
    this->outputFile = config["outputFile"].get<std::string>();
  }
}

void JsonEngineManager::injectionPointEndedCallBack(ICommunicator_ptr /**comm**/,
                                                    std::string id,
                                                    InjectionPointType type,
                                                    std::string stageVal) {
  if (type == InjectionPointType::End || type == InjectionPointType::Single) {
    pop(4);
  } else {
    pop(2);
  }
}

void JsonEngineManager::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                      std::string packageName,
                                                      std::string id,
                                                      InjectionPointType type,
                                                      std::string stageVal) {
  setComm(comm);
  json ip;
  json stage;

  stage["name"] = id;
  stage["package"] = packageName;
  stage["type"] = InjectionPointTypeUtils::getType(type, stageVal);
  stage["stageId"] = stageVal;
  stage["children"] = json::array();
  stage["node"] = "InjectionPointStage";

  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    json j;
    j["node"] = "InjectionPoint";
    j["name"] = id;
    j["id"] = getId();
    j["package"] = packageName;
    j["children"] = json::array();

    stage["id"] = getId();
    j["children"].push_back(stage);
    push(j, json::json_pointer("/children/0/children"));
    // Now we are pointing to the child node, so all test stuff there.
  } else {
    stage["id"] = getId();
    push(stage, json::json_pointer("/children"));
  }
}

void JsonEngineManager::testStartedCallBack(ICommunicator_ptr comm,
                                            std::string packageName,
                                            std::string testName,
                                            bool internal) {
  setComm(comm);
  json j;
  j["id"] = getId();
  j["name"] = testName;
  j["package"] = packageName;
  j["internal"] = internal;
  j["node"] = "Test";
  j["children"] = json::array();
  push(j, json::json_pointer("/children"));
}

void JsonEngineManager::testFinishedCallBack(ICommunicator_ptr /**comm**/,
                                             bool result_) {
  pop(2);
}

void JsonEngineManager::unitTestStartedCallBack(ICommunicator_ptr comm,
                                                std::string packageName,
                                                std::string unitTestName) {
  json j;
  setComm(comm);
  j["id"] = getId();
  j["node"] = "UnitTest";
  j["name"] = unitTestName;
  j["package"] = packageName;
  j["children"] = json::array();
  j["results"] = json::array();
  push(j, json::json_pointer("/children"));
}
void JsonEngineManager::dataTypeStartedCallBack(ICommunicator_ptr comm ,
                                                std::string variableName,
                                                long long dtype) {
  json j;
  setComm(comm);
  j["id"] = getId();
  j["node"] = "DataType";
  j["name"] = variableName;
  j["dtype"] = dtype;
  j["children"] = json::array();
  j["results"] = json::array();
  push(j, json::json_pointer("/children"));
}

void JsonEngineManager::dataTypeEndedCallBack(ICommunicator_ptr /** comm **/,
                                              std::string variableName) {
  pop(2);
}

void JsonEngineManager::unitTestFinishedCallBack(ICommunicator_ptr comm,
                                                 IUnitTest* tester) {
  // pop the children node
  pop(1);

  // push to the results node
  append(json::json_pointer("/results"));
  for (auto it : tester->getResults()) {
    Put(std::get<0>(it), std::get<2>(it));
  }

  // pop the results and the unit-test itself.
  pop(2);
}

Nodes::IRootNode* JsonEngineManager::readFromFile(std::string file,
                                                  long& idCounter) {
  return VnV::VNVPACKAGENAME::Engines::JsonReader::parse(file, idCounter);
}

std::string JsonEngineManager::print() { return "VnV Json Engine Manager"; }

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
