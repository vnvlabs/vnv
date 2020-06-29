#include "plugins/engines/json/JsonEngineManager.h"
#include <chrono>
#include <fstream>
#include <iostream>

#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "plugins/engines/json/JsonOutputReader.h"
#include "base/Runtime.h"

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
INJECTION_ENGINE(json) {
  return new VnV::PACKAGENAME::Engines::JsonEngineManager();
}

namespace VnV {
namespace PACKAGENAME {
namespace Engines {

#define LTypes X(double) X(int) X(bool) X(float) X(long) X(std::string) X(json)
#define X(type)                                                              \
  void JsonEngineManager::Put(VnV_Comm /**comm**/, std::string variableName, \
                              const type& value) {                           \
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

    std::string
    JsonEngineManager::getId() {
  return std::to_string(id++);
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

  ptr = json::json_pointer("/children");
}

void JsonEngineManager::Log(VnV_Comm /**comm**/, const char* package, int stage,
                            std::string level, std::string message) {
  json log = json::object();
  log["package"] = package;
  log["stage"] = stage;
  log["level"] = level;
  log["message"] = message;
  log["node"] = "Log";
  log["id"] = getId();
  log["name"] = getId();
  append(log);
}

nlohmann::json JsonEngineManager::getConfigurationSchema() {
  return __json_engine_schema__;
}

void JsonEngineManager::finalize() {

  //TODO -- Could cut this down to just what we need. Right now, we export
  //everthing.
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

void JsonEngineManager::injectionPointEndedCallBack(VnV_Comm /**comm**/,
                                                    std::string id,
                                                    InjectionPointType type,
                                                    std::string stageVal) {
  if (type == InjectionPointType::End || type == InjectionPointType::Single) {
    pop(4);
  } else {
    pop(2);
  }
}

void JsonEngineManager::injectionPointStartedCallBack(VnV_Comm /**comm**/,
                                                      std::string id,
                                                      InjectionPointType type,
                                                      std::string stageVal) {
  json ip;
  json stage;

  stage["name"] = id;
  stage["package"] = "TODO";
  stage["type"] = InjectionPointTypeUtils::getType(type, stageVal);
  stage["stageId"] = stageVal;
  stage["children"] = json::array();
  stage["node"] = "InjectionPointStage";

  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    json j;
    j["node"] = "InjectionPoint";
    j["name"] = id;
    j["id"] = getId();
    j["package"] = "TODO";
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

void JsonEngineManager::testStartedCallBack(VnV_Comm /**comm**/,
                                            std::string testName) {
  json j;
  j["id"] = getId();
  j["name"] = testName;
  j["package"] = "TODO";
  j["node"] = "Test";
  j["children"] = json::array();
  push(j, json::json_pointer("/children"));
}

void JsonEngineManager::testFinishedCallBack(VnV_Comm /**comm**/,
                                             bool result_) {
  pop(2);
}

void JsonEngineManager::unitTestStartedCallBack(VnV_Comm /**comm**/,
                                                std::string unitTestName) {
  json j;

  j["id"] = getId();
  j["node"] = "UnitTest";
  j["name"] = unitTestName;
  j["package"] = "TODO";
  j["children"] = json::array();
  j["results"] = json::array();
  push(j, json::json_pointer("/children"));
}
void JsonEngineManager::dataTypeStartedCallBack(VnV_Comm /** comm **/,
                                             std::string variableName,std::string dtype) {
  json j;

  j["id"] = getId();
  j["node"] = "DataType";
  j["name"] = variableName;
  j["dtype"] = dtype;
  j["package"] = "TODO";
  j["children"] = json::array();
  j["results"] = json::array();
  push(j, json::json_pointer("/children"));
}

void JsonEngineManager::dataTypeEndedCallBack(VnV_Comm /** comm **/, std::string variableName) {
  pop(2);
}

void JsonEngineManager::unitTestFinishedCallBack(VnV_Comm comm,
                                                 IUnitTest* tester) {
  // pop the children node
  pop(1);

  // push to the results node
  append(json::json_pointer("/results"));
  for (auto it : tester->getResults()) {
    Put(comm, std::get<0>(it), std::get<2>(it));
  }

  // pop the results and the unit-test itself.
  pop(2);
}

Nodes::IRootNode* JsonEngineManager::readFromFile(std::string file) {
  return VnV::PACKAGENAME::Engines::JsonReader::parse(file);
}

std::string JsonEngineManager::print() { return "VnV Json Engine Manager"; }

}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
