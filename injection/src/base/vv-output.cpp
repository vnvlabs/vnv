
/** @file vv-output.cpp **/
#include "vv-output.h"

#include <iostream>
#include <string>

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

void EngineStore::setEngineManager(std::string type, json& config) {
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    manager = it->second();
    manager->_set(config);
    return;
  }

  VnV_Error("Invalid Engine Name: {}", type);
  printAvailableEngines();

  throw "Invalid Engine Name";
}

void EngineStore::printAvailableEngines() {
  VnV_Info("Available Engines:");
  for (auto it : registeredEngines) {
    VnV_Info("\t {}", it.first);
  }
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

void EngineStore::registerEngine(std::string name,
                                 engine_register_ptr* engine_ptr) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
}

void VnV_registerEngine(std::string name, engine_register_ptr r) {
  EngineStore::getEngineStore().registerEngine(name, r);
}

IOutputEngine::~IOutputEngine() {}
OutputEngineManager::~OutputEngineManager() {}
