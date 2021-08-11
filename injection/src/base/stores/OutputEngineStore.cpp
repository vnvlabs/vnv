
/** @file OutputEngineStore.cpp Implementation of the OutputEngineStore as
 *defined in base/OutputEngineStore.h"
 **/

#include "base/stores/OutputEngineStore.h"

#include "base/exceptions.h"
#include "base/Runtime.h"
#include "c-interfaces/Logging.h"
#include "base/Runtime.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

bool OutputEngineStore::isInitialized() { return initialized; }

void OutputEngineStore::setEngineManager(ICommunicator_ptr world, std::string type, json& config) {
   
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    manager.reset(it->second());
    manager->set(world, config,type,false);
    initialized = true;
    engineName = type;
    return;
  }

  throw VnVExceptionBase("Invalid Engine Name");
}

void OutputEngineStore::printAvailableEngines() {
  for (auto it : registeredEngines) {
    VnV_Info(VNVPACKAGENAME, "%s", it.first.c_str());
  }
}

OutputEngineStore::OutputEngineStore() {}

void OutputEngineStore::print() {
  printAvailableEngines();
  if (manager != nullptr) {
    manager->print();
  }
 }

void OutputEngineStore::registerEngine(std::string name,
                                       engine_register_ptr* engine_ptr) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
}

std::shared_ptr<Nodes::IRootNode> OutputEngineStore::readFile(std::string filename,
                                              std::string engineType,
                                              json& config) {
  auto it = registeredEngines.find(engineType);
  if (it != registeredEngines.end()) {
    std::unique_ptr<OutputEngineManager> engine(it->second());
    ICommunicator_ptr ptr = CommunicationStore::instance().worldComm();
    engine->set(ptr,config,engineType,true);
    std::shared_ptr<Nodes::IRootNode> rootNode = engine->readFromFile(filename, idCounter);
    engine->finalize(CommunicationStore::instance().worldComm());
    return rootNode;
  }
  throw VnVExceptionBase("Invalid Engine Name");
}

OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw VnVExceptionBase("Engine Not Initialized Error");
}

BaseStoreInstance(OutputEngineStore)
