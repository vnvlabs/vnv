
/** @file vv-output.cpp **/
#include "vv-output.h"

#include <iostream>
#include <string>
#include "VnV.h"
#include "VnV-Interfaces.h"
#include "vv-logging.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

static json __default_configuration_schema__ = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "http://rnet-tech.net/vv.schema.json",
  "title": "Basic VV RunTime Schema",
  "description": "Schema for a VnV runtime Configuration file",
  "type": "object"
})"_json;


VariableEnum VariableEnumFactory::fromString(std::string s) {
        if (s.compare("Double") == 0) return VariableEnum::Double;
        else if (s.compare("String") == 0) return VariableEnum::Long;
        else if (s.compare("Int") == 0) return VariableEnum::Int;
        else if (s.compare("Float") == 0) return VariableEnum::Float;
        else if (s.compare("Long") == 0) return VariableEnum::Long;

        s= "VariableEnumFactory::fromString: Unknown Variable Type" + s;
        throw s.c_str();
}

std::string VariableEnumFactory::toString(VariableEnum e) {
        switch (e) {
          case VariableEnum::Double: return "Double";
          case VariableEnum::Long: return "Long";
          case VariableEnum::Float: return "Float";
          case VariableEnum::Int: return "Long";
          case VariableEnum::String: return "String";
        }

        throw "VariableEnumFactory::toString: Unhandled Variable Enum Type";
}

void IOutputEngine::Put(std::string /*variableName*/, double& /**value**/){throw "Engine Does not support type double";}
void IOutputEngine::Put(std::string /*variableName*/, int& /**value**/){throw "Engine Does not support type int";}
void IOutputEngine::Put(std::string /*variableName*/, float& /**value**/){throw "Engine Does not support type float";}
void IOutputEngine::Put(std::string /*variableName*/, long& /**value**/){throw "Engine Does not support type long";}
void IOutputEngine::Put(std::string /*variableName*/, std::string& /**value**/){throw "Engine Does not support type string";}
void IOutputEngine::Log(const char *, int, std::string, std::string) { throw "Engine does not support in engine logging";}

json OutputEngineManager::getConfigurationSchema() {
    return __default_configuration_schema__;
}

void OutputEngineManager::_set(json& inputjson) {
  json schema = getConfigurationSchema();

  if (!schema.empty()) {
    json_validator validator;
    validator.set_root_schema(schema);
    validator.validate(inputjson);
  } 
  set(inputjson);
}

bool EngineStore::isInitialized(){
    return initialized;
}

void EngineStore::setEngineManager(std::string type, json& config) {
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    manager = it->second();
    manager->_set(config);
    initialized = true;
    engineName = type;
    return;
  }

  //VnV_Error("Invalid Engine Name: {}", type);
  printAvailableEngines();

  throw "Invalid Engine Name";
}

void EngineStore::printAvailableEngines() {
  int a = VnV_BeginStage("Available Engines:");
  for (auto it : registeredEngines) {
     VnV_Info("%s", it.first.c_str());
  }
  VnV_EndStage(a);
}

void OutputEngineManager::print() {
    VnV_Info("Print not implemented for this Output Engine Manager");
}

OutputEngineManager* EngineStore::getEngineManager() {
  if (manager != nullptr) return manager;
  throw "Engine Not Initialized Error";
}

EngineStore::EngineStore() {}

EngineStore& EngineStore::getEngineStore() {
  static EngineStore* engine = new EngineStore();
  return *engine;
}

void EngineStore::print() {
    int b = VnV_BeginStage("Output Engine Configuration");
    printAvailableEngines();
    if ( manager != nullptr) {
       int a = VnV_BeginStage("Chosen Engine: %s ", engineName.c_str());
       manager->print();
       VnV_EndStage(a);
    }
    VnV_EndStage(b);


}

void EngineStore::registerEngine(std::string name,
                                 engine_register_ptr* engine_ptr) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
}

void VnV_registerEngine(std::string name, engine_register_ptr r) {
  EngineStore::getEngineStore().registerEngine(name, r);
}

IOutputEngine::~IOutputEngine() {}
OutputEngineManager::~OutputEngineManager() {}
