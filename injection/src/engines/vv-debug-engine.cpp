
/** @file vv-debug-engine.cpp **/

#include "vv-debug-engine.h"

#include <iostream>

#include "vv-logging.h"
#include "vv-output.h"
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

void DebugEngine::Put(std::string variableName, double& value) {
  printf("DEBUG ENGINE PUT %s = %f", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, int& value) {
  printf("DEBUG ENGINE PUT %s = %d", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, float& value) {
  printf("DEBUG ENGINE PUT %s = %f", variableName.c_str(), value);
}
void DebugEngine::Put(std::string variableName, long& value) {
  printf("DEBUG ENGINE PUT %s = %ld", variableName.c_str(), value);
}

void DebugEngine::Put(std::string variableName, std::string& value) {
  printf("DEBUG ENGINE PUT %s = %s", variableName.c_str(), value.c_str());
}

void DebugEngine::Log(const char *package, int stage, LogLevel level, std::string message) {
    std::string s = getIndent(stage);
    printf("%s[%s:%s]: %s", s.c_str(), package, logLevelToString(level).c_str(), message.c_str() );
}

void DebugEngine::Define(VariableEnum type, std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE %s = %s", variableName.c_str(), VariableEnumFactory::toString(type).c_str());
}

DebugEngineWrapper::DebugEngineWrapper() {}

json DebugEngineWrapper::getConfigurationSchema() {
	return __debug_engine_schema__;
}


void DebugEngineWrapper::finalize() { VnV_Info("DEBUG ENGINE: FINALIZE"); }

void DebugEngineWrapper::set(json& config) {
  VnV_Info("DEBUG ENGINE WRAPPER Init with file %s", config.dump().c_str());
  
  debugEngine = new DebugEngine();
}

void DebugEngineWrapper::endInjectionPoint(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE End Injection Point %s : %s ", id.c_str(), stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::startInjectionPoint(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Start Injection Point %s : %s ", id.c_str(), stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::startTest(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Start Test %s : %s ", id.c_str(), stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::stopTest(bool result_) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Stop Test. Test Was Successful-> %s", result_);
  } else {
    throw "Engine not initialized";
  }
}

IOutputEngine* DebugEngineWrapper::getOutputEngine() { return debugEngine; }

extern "C" {
OutputEngineManager* DebugEngineBuilder() { return new DebugEngineWrapper(); }
}

class Debug_engine_proxy {
 public:
  Debug_engine_proxy() { VnV_registerEngine("debug", DebugEngineBuilder); }
};

Debug_engine_proxy debug_engine_proxy;
