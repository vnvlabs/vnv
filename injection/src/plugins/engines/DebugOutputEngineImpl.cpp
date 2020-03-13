
/** @file DebugOutputEngineImpl.cpp **/

#include "plugins/engines/DebugOutputEngineImpl.h"
#include "c-interfaces/Logging.h"
#include <iostream>

using namespace VnV;

static json __debug_engine_schema__ = R"(
{
	"$schema": "http://json-schema.org/draft-07/schema#",
	"$id": "http://rnet-tech.net/vv-debug-schema.json",
	"title": "Debug Engine Input Schema",
	"description": "Schema for the debug engine",
    "type": "object"
}
)"_json;


DebugEngine::DebugEngine() {}

void DebugEngine::Put(std::string variableName, const double& value) {
  printf("DEBUG ENGINE PUT %s = %f\n", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, const int& value) {
  printf("DEBUG ENGINE PUT %s = %d\n", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, const float& value) {
  printf("DEBUG ENGINE PUT %s = %f\n", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, const long& value) {
  printf("DEBUG ENGINE PUT %s = %ld\n", variableName.c_str(), value);
}

void DebugEngine::Put(std::string variableName, const json& value) {
 printf("DEBUG ENGINE PUT %s = %s\n" , variableName.c_str(), value.dump(3).c_str());
}

void DebugEngine::Put(std::string variableName, const std::string& value) {
  printf("DEBUG ENGINE PUT %s = %s\n", variableName.c_str(), value.c_str());
}

void DebugEngine::Log(const char *package, int stage,  std::string level, std::string message) {

    std::string s = getIndent(stage);
    printf("%s[%s:%s]: %s\n", s.c_str(), package, level.c_str(), message.c_str() );
}

void DebugEngine::Define(VariableEnum type, std::string variableName) {
  printf("DEBUG ENGINE DEFINE %s = %s\n", variableName.c_str(), VariableEnumFactory::toString(type).c_str());
}

DebugEngineWrapper::DebugEngineWrapper() {}

json DebugEngineWrapper::getConfigurationSchema() {
	return __debug_engine_schema__;
}


void DebugEngineWrapper::finalize() { VnV_Info("DEBUG ENGINE: FINALIZE"); }

void DebugEngineWrapper::set(json& config) {
  printf("DEBUG ENGINE WRAPPER Init with file %s\n", config.dump().c_str());
  
  debugEngine = new DebugEngine();
}

void DebugEngineWrapper::injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageVal) {
  if (debugEngine) {
    printf("DEBUG ENGINE End Injection Point %s : %s \n", id.c_str(), InjectionPointTypeUtils::getType(type,stageVal).c_str());
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::injectionPointStartedCallBack(std::string id, InjectionPointType type, std::string stageVal) {
  if (debugEngine) {
    printf("DEBUG ENGINE Start Injection Point %s : %s \n", id.c_str(),InjectionPointTypeUtils::getType(type,stageVal).c_str());
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::testStartedCallBack(std::string testName) {
  if (debugEngine) {
    printf("DEBUG ENGINE Start Test %s \n", testName.c_str() );
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::testFinishedCallBack(bool result_) {
  if (debugEngine) {
    printf("DEBUG ENGINE Stop Test. Test Was Successful-> %d\n", result_);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::documentationStartedCallBack(std::string pname, std::string id) {
  if (debugEngine) {
    printf("DEBUG ENGINE Start Docs %s:%s \n", pname.c_str(),id.c_str() );
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::documentationEndedCallBack(std::string pname, std::string id) {
  if (debugEngine) {
    printf("DEBUG ENGINE Stop Docs %s:%s", pname.c_str(),id.c_str());
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::unitTestStartedCallBack(std::string unitTestName) {
    if  (debugEngine) {
        printf("DEBUG ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
    } else {
        throw "Engine not initialized";
    }
}

void DebugEngineWrapper::unitTestFinishedCallBack(std::map<std::string, bool> &results){
    if (debugEngine) {
        printf("Test Results\n");
        bool suiteSuccessful = true;
        for ( auto it : results ) {
            printf("\t%s : %s\n", it.first.c_str(), (it.second) ? "Successful" : "Failed");
            if (suiteSuccessful && !it.second) {
                suiteSuccessful = false;
           }
        }
        printf("DEBUG ENGINE Test Suite Completed : %s\n", (suiteSuccessful) ? "Successfully" : "Unsuccessfully");
    } else {
        throw "Engine Not Initialized";
    }
}

IOutputEngine* DebugEngineWrapper::getOutputEngine() { return debugEngine; }

OutputEngineManager* DebugEngineBuilder() { return new DebugEngineWrapper(); }
