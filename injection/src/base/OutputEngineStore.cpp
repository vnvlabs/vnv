
/** @file OutputEngineStore.cpp Implementation of the OutputEngineStore as
 *defined in base/OutputEngineStore.h"
 **/

#include "base/OutputEngineStore.h"

#include "base/exceptions.h"
#include "c-interfaces/Logging.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

bool OutputEngineStore::isInitialized() { return initialized; }

void OutputEngineStore::setEngineManager(std::string type, json& config) {
  if (manager != nullptr) {
    manager->finalize();
    manager.reset();
  }

  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    manager.reset(it->second());
    manager->set(config);
    initialized = true;
    engineName = type;
    return;
  }

  throw VnVExceptionBase("Invalid Engine Name");
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
  if (manager != nullptr) {
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

Nodes::IRootNode* OutputEngineStore::readFile(std::string filename,
                                              std::string engineType,
                                              json& config) {
  auto it = registeredEngines.find(engineType);
  if (it != registeredEngines.end()) {
    std::unique_ptr<OutputEngineManager> engine(it->second());
    engine->set(config);
    Nodes::IRootNode* rootNode = engine->readFromFile(filename, idCounter);
    engine->finalize();
    return rootNode;
  }
  throw VnVExceptionBase("Invalid Engine Name");
}

OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw VnVExceptionBase("Engine Not Initialized Error");
}
