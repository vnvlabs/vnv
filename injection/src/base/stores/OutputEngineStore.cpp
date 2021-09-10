
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
    manager.reset((*it->second)());
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
 
json OutputEngineStore::listReaders() {
  json a = json::array();
  for (auto it : registeredReaders) {
    a.push_back(it.first);
  }
  return a;
}

json OutputEngineStore::schema() {
    

   nlohmann::json oneof = json::array();
    for (auto &it :  registeredEngines) {
      nlohmann::json m = R"({"type":"object"})"_json;
      
      nlohmann::json properties = json::object();
      
      nlohmann::json package = json::object();
      package["const"] = it.first;
      properties["type"] = package;
      properties["config"] = R"({"type":"object"})"_json; //todo 
    
      m["properties"] = properties;
      m["required"] = R"(["type"])"_json;
      oneof.push_back(m);  
    }
    
   if (oneof.size() > 0 ) {
      nlohmann::json ret = json::object();
      ret["oneOf"] = oneof;
      return ret;
    } else {
      return R"({"const" : false})"_json;
    }

}

void OutputEngineStore::registerEngine(std::string name,
                                       engine_register_ptr engine_ptr) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
}

void OutputEngineStore::registerReader(std::string name,
                                       engine_reader_ptr engine_ptr) {
  registeredReaders.insert(std::make_pair(name, engine_ptr));
}


std::shared_ptr<Nodes::IRootNode> OutputEngineStore::readFile(std::string filename,
                                              std::string engineType,
                                              json& config) {
  auto it = registeredReaders.find(engineType);
  
  if (it != registeredReaders.end()) {
    return (*it->second)(filename, idCounter, config);
  }

  throw VnVExceptionBase("Invalid Engine Reader");
}

OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw VnVExceptionBase("Engine Not Initialized Error");
}

BaseStoreInstance(OutputEngineStore)
