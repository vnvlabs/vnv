
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
	"type": "object",
	"additionalProperties": false
}
)"_json;


DebugEngine::DebugEngine() {}

void DebugEngine::Put(std::string variableName, double& value) {
  VnV_Info("DEBUG ENGINE PUT {} = {}", variableName, value);
}
void DebugEngine::Put(std::string variableName, int& value) {
  VnV_Info("DEBUG ENGINE PUT {} = {}", variableName, value);
}
void DebugEngine::Put(std::string variableName, float& value) {
  VnV_Info("DEBUG ENGINE PUT {} = {}", variableName, value);
}
void DebugEngine::Put(std::string variableName, long& value) {
  VnV_Info("DEBUG ENGINE PUT {} = {}", variableName, value);
}

void DebugEngine::Put(std::string variableName, std::string& value) {
  VnV_Info("DEBUG ENGINE PUT {} = {}", variableName, value);
}

void DebugEngine::DefineDouble(std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE {} = {}", variableName, "double");
}

void DebugEngine::DefineFloat(std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE {} = {}", variableName, "float");
}

void DebugEngine::DefineString(std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE {} = {}", variableName, "string");
}

void DebugEngine::DefineInt(std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE {} = {}", variableName, "int");
}

void DebugEngine::DefineLong(std::string variableName) {
  VnV_Info("DEBUG ENGINE DEFINE {} = {}", variableName, "long");
}

DebugEngineWrapper::DebugEngineWrapper() {}

json DebugEngineWrapper::getConfigurationSchema() {
	return __debug_engine_schema__;
}


void DebugEngineWrapper::finalize() { VnV_Info("DEBUG ENGINE: FINALIZE"); }

void DebugEngineWrapper::set(json& config) {
  VnV_Info("DEBUG ENGINE WRAPPER Init with file {}", config.dump());
  
  debugEngine = new DebugEngine();
}

void DebugEngineWrapper::endInjectionPoint(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE End Injection Point {} : {} ", id, stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::startInjectionPoint(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Start Injection Point {} : {} ", id, stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::startTest(std::string id, int stageVal) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Start Test {} : {} ", id, stageVal);
  } else {
    throw "Engine not initialized";
  }
}

void DebugEngineWrapper::stopTest(bool result_) {
  if (debugEngine) {
    VnV_Info("DEBUG ENGINE Stop Test. Test Was Successful-> {}", result_);
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
