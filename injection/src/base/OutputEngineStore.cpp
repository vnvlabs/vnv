
/** @file vv-output.cpp **/

#include <iostream>
#include <string>
#include "base/OutputEngineStore.h"

#include "c-interfaces/Logging.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;


bool OutputEngineStore::isInitialized(){
    return initialized;
}

void OutputEngineStore::setEngineManager(std::string type, json& config) {
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

void OutputEngineStore::printAvailableEngines() {
  int a = VnV_BeginStage("Available Engines:");
  for (auto it : registeredEngines) {
     VnV_Info("%s", it.first.c_str());
  }
  VnV_EndStage(a);
}


OutputEngineStore::OutputEngineStore() {}

OutputEngineStore& OutputEngineStore::getOutputEngineStore() {
  static OutputEngineStore engine;
  return engine;
}

void OutputEngineStore::print() {
    int b = VnV_BeginStage("Output Engine Configuration");
    printAvailableEngines();
    if ( manager != nullptr) {
       int a = VnV_BeginStage("Chosen Engine: %s ", engineName.c_str());
       manager->print();
       VnV_EndStage(a);
    }
    VnV_EndStage(b);


}

void OutputEngineStore::registerEngine(std::string name,
                                 engine_register_ptr* engine_ptr) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
}


OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager;
  throw "Engine Not Initialized Error";
}
