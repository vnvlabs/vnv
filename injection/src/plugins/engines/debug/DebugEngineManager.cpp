
/** @file DebugOutputEngineImpl.cpp **/

#include "plugins/engines/debug/DebugEngineManager.h"

#include <iostream>

#include "base/exceptions.h"
#include "c-interfaces/Logging.h"

using nlohmann::json;

static json __debug_engine_schema__ = R"(
{
	"$schema": "http://json-schema.org/draft-07/schema#",
	"$id": "http://rnet-tech.net/vv-debug-schema.json",
	"title": "Debug Engine Input Schema",
	"description": "Schema for the debug engine",
    "type": "object"
}
)"_json;

INJECTION_ENGINE(VNVPACKAGENAME, debug) {
  return new VnV::VNVPACKAGENAME::Engines::DebugEngineManager();
}

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

DebugEngineManager::DebugEngineManager() {}

DebugEngineManager::~DebugEngineManager() {}

void DebugEngineManager::Put(std::string variableName,
         IDataType_ptr data,
         const MetaData& m ) {
  printf("DEBUG ENGINE DATATYPE PUT START\n");
  data->Put(this);
  printf("DEBUG ENGINE DATATYPE PUT End\n");
}

void DebugEngineManager::Put( std::string variableName,
                             const double& value,const MetaData& m) {
  printf("DEBUG ENGINE PUT %s = %f\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(std::string variableName,
                             const long long& value,const MetaData& m) {
  printf("DEBUG ENGINE PUT %s = %ld\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(std::string variableName,
                             const nlohmann::json& value,const MetaData& m) {
  printf("DEBUG ENGINE PUT %s = %s\n", variableName.c_str(),
         value.dump(3).c_str());
}

void DebugEngineManager::Put(std::string variableName,
                             const std::string& value,const MetaData& m) {
  printf("DEBUG ENGINE PUT %s = %s\n", variableName.c_str(), value.c_str());
}

void DebugEngineManager::Put(std::string variableName,
                             const bool& value,const MetaData& m) {
  printf("DEBUG ENGINE PUT %s = %d\n", variableName.c_str(), value);
}

void DebugEngineManager::Log(ICommunicator_ptr comm, const char* package, int stage,
                             std::string level, std::string message) {
  printf("[%s:%s]: %s\n", package, level.c_str(), message.c_str());
}

nlohmann::json DebugEngineManager::getConfigurationSchema() {
  return __debug_engine_schema__;
}

void DebugEngineManager::finalize(ICommunicator_ptr worldComm) {
  VnV_Info(VNVPACKAGENAME, "DEBUG ENGINE: FINALIZE");
}

void DebugEngineManager::setFromJson(ICommunicator_ptr worldComm, nlohmann::json& config) {
  printf("DEBUG ENGINE WRAPPER Init with file %s\n", config.dump().c_str());
}

void DebugEngineManager::injectionPointEndedCallBack(
                                                     std::string id,
                                                     InjectionPointType type,
                                                     std::string stageVal) {
  printf("DEBUG ENGINE End Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
}

void DebugEngineManager::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                       std::string packageName,
                                                       std::string id,
                                                       InjectionPointType type,
                                                       std::string stageVal) {
  printf("DEBUG ENGINE Start Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
}

void DebugEngineManager::testStartedCallBack(
                                             std::string packageName,
                                             std::string testName,
                                             bool internal) {
  printf("DEBUG ENGINE Start Test %s \n", testName.c_str());
}

void DebugEngineManager::testFinishedCallBack( bool result_) {
  printf("DEBUG ENGINE Stop Test. Test Was Successful-> %d\n", result_);
}


void DebugEngineManager::unitTestStartedCallBack(ICommunicator_ptr comm,
                                                 std::string packageName,
                                                 std::string unitTestName) {
  printf("DEBUG ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
}

void DebugEngineManager::unitTestFinishedCallBack(
                                                  IUnitTest* tester) {
  printf("Test Results\n");
  bool suiteSuccessful = true;
  for (auto it : tester->getResults()) {
    printf("\t%s : %s\n", std::get<0>(it).c_str(),
           std::get<2>(it) ? "Successful" : "Failed");
    if (!std::get<2>(it)) {
      printf("\t\t%s\n", std::get<1>(it).c_str());
      suiteSuccessful = false;
    }
  }
  printf("DEBUG ENGINE Test Suite Completed : %s\n",
         (suiteSuccessful) ? "Successfully" : "Unsuccessfully");
}

Nodes::IRootNode* DebugEngineManager::readFromFile(std::string file, long&) {
  throw VnVExceptionBase("The debug engine does not support reading from file");
}

std::string DebugEngineManager::print() { return "VnV Debug Engine Manager"; }

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
