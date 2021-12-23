
/** @file OutputEngineStore.cpp Implementation of the OutputEngineStore as
 *defined in base/OutputEngineStore.h"
 **/

#include "base/stores/OutputEngineStore.h"

#include "base/Runtime.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

bool OutputEngineStore::isInitialized() { return initialized; }

void OutputEngineStore::setEngineManager(ICommunicator_ptr world,
                                         std::string type, json& config) {
  
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    
    try {
    manager.reset((*it->second)());
    } catch (std::exception &e) {
      HTHROW INJECTION_EXCEPTION("Engine init failed:  %s", e.what());
    } catch (...) {
      HTHROW INJECTION_EXCEPTION_("Unknown third party exception occured during engine init:");
    }

    if (manager == nullptr) {
      std::abort();
    }
    
    manager->set(world, config, type, false);
    initialized = true;
    engineName = type;
    return;
  }
  HTHROW INJECTION_EXCEPTION("Invalid Engine Name %s", type.c_str());
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
  for (auto& it : registeredEngines) {
    nlohmann::json m = R"({"type":"object"})"_json;

    nlohmann::json properties = json::object();

    nlohmann::json package = json::object();
    package["const"] = it.first;
    properties["type"] = package;
    properties["config"] = R"({"type":"object"})"_json;  // todo

    m["properties"] = properties;
    m["required"] = R"(["type"])"_json;
    oneof.push_back(m);
  }

  if (oneof.size() > 0) {
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

std::shared_ptr<Nodes::IRootNode> OutputEngineStore::readFile(
    std::string filename, std::string engineType, json& config, bool async) {
  
  auto it = registeredReaders.find(engineType);
  if (it != registeredReaders.end()) {
    return (*it->second)(filename, idCounter, config, async);
  }
  throw INJECTION_EXCEPTION("Invalid Engine Reader %s ", engineType.c_str());
}

OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw INJECTION_BUG_REPORT_("Engine Not Initialized Error");
}

BaseStoreInstance(OutputEngineStore)
