
/** @file vv-output.cpp **/

#include <iostream>
#include <string>
#include "VnV.h"
#include "base/vv-logging.h"
#include "base/vv-output.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;


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


EngineStore::EngineStore() {}

EngineStore& EngineStore::getEngineStore() {
  static EngineStore engine;
  return engine;
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

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  EngineStore::getEngineStore().registerEngine(name, r);
}

