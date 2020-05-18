
/** @file DebugOutputEngineImpl.cpp **/

#include "plugins/engines/debug/DebugEngineManager.h"
#include "c-interfaces/Logging.h"
#include <iostream>
#include "base/exceptions.h"

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

INJECTION_ENGINE(debug) {
   return new VnV::PACKAGENAME::Engines::DebugEngineManager();
}

namespace VnV {
namespace PACKAGENAME {
namespace Engines {

OutputEngineManager* DebugEngineBuilder() { return new DebugEngineManager(); }

DebugEngineManager::DebugEngineManager() {}

DebugEngineManager::~DebugEngineManager() {}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const double &value) {
    printf("DEBUG ENGINE PUT %s = %f\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const int &value){
    printf("DEBUG ENGINE PUT %s = %d\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const float &value){
    printf("DEBUG ENGINE PUT %s = %f\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const long &value){
    printf("DEBUG ENGINE PUT %s = %ld\n", variableName.c_str(), value);
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const nlohmann::json &value){
    printf("DEBUG ENGINE PUT %s = %s\n" , variableName.c_str(), value.dump(3).c_str());
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const std::string &value){
    printf("DEBUG ENGINE PUT %s = %s\n", variableName.c_str(), value.c_str());
}

void DebugEngineManager::Put(VnV_Comm comm, std::string variableName, const bool &value) {
    printf("DEBUG ENGINE PUT %s = %d\n", variableName.c_str(), value);
}

void DebugEngineManager::Log(VnV_Comm comm, const char *package, int stage, std::string level, std::string message){
    printf("[%s:%s]: %s\n", package, level.c_str(), message.c_str() );
}

nlohmann::json DebugEngineManager::getConfigurationSchema(){
    return __debug_engine_schema__;
}

void DebugEngineManager::finalize(){
    VnV_Info("DEBUG ENGINE: FINALIZE");
}

void DebugEngineManager::setFromJson(nlohmann::json &config){
    printf("DEBUG ENGINE WRAPPER Init with file %s\n", config.dump().c_str());
}

void DebugEngineManager::injectionPointEndedCallBack(VnV_Comm comm, std::string id, InjectionPointType type, std::string stageVal){
    printf("DEBUG ENGINE End Injection Point %s : %s \n", id.c_str(), InjectionPointTypeUtils::getType(type,stageVal).c_str());
}

void DebugEngineManager::injectionPointStartedCallBack(VnV_Comm comm, std::string id, InjectionPointType type, std::string stageVal){
    printf("DEBUG ENGINE Start Injection Point %s : %s \n", id.c_str(),InjectionPointTypeUtils::getType(type,stageVal).c_str());
}

void DebugEngineManager::testStartedCallBack(VnV_Comm comm, std::string testName){
    printf("DEBUG ENGINE Start Test %s \n", testName.c_str() );
}

void DebugEngineManager::testFinishedCallBack(VnV_Comm comm, bool result_){
    printf("DEBUG ENGINE Stop Test. Test Was Successful-> %d\n", result_);
}

void DebugEngineManager::unitTestStartedCallBack(VnV_Comm comm, std::string unitTestName){
    printf("DEBUG ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
}

void DebugEngineManager::unitTestFinishedCallBack(VnV_Comm comm, std::map<std::string, bool> &results){
    printf("Test Results\n");
    bool suiteSuccessful = true;
    for ( auto it : results ) {
        printf("\t%s : %s\n", it.first.c_str(), (it.second) ? "Successful" : "Failed");
        if (suiteSuccessful && !it.second) {
            suiteSuccessful = false;
        }
    }
    printf("DEBUG ENGINE Test Suite Completed : %s\n", (suiteSuccessful) ? "Successfully" : "Unsuccessfully");
}

Nodes::IRootNode *DebugEngineManager::readFromFile(std::string file) {
    throw VnVExceptionBase("The debug engine does not support reading from file");
}

std::string DebugEngineManager::print() {
    return "VnV Debug Engine Manager";
}

}
}
}
