
/** @file OutputEngineStore.cpp Implementation of the OutputEngineStore as
 *defined in base/OutputEngineStore.h"
 **/

#include "base/stores/OutputEngineStore.h"

#include "base/Runtime.h"
#include "shared/exceptions.h"
#include "common-interfaces/all.h"

using namespace VnV;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

bool OutputEngineStore::isInitialized() { return initialized; }

void OutputEngineStore::setEngineManager(ICommunicator_ptr world, std::string type, json& config) {
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    try {
      manager.reset((*it->second)());
    } catch (std::exception& e) {
      throw INJECTION_EXCEPTION("Engine init failed:  %s", e.what());
    }

    if (manager == nullptr) {
      std::abort();
    }

    json schema = json::parse((*registeredEngineSchema[type])());
    try {
      if (!schema.empty()) {
        json_validator validator;
        validator.set_root_schema(schema);
        validator.validate(config);
        manager->set(world, config, type);
        initialized = true;
        engineName = type;
      }
      return;
    } catch (std::exception& e) {
      throw INJECTION_EXCEPTION_("Invalid Engine Schema");
    }
  }
  throw INJECTION_EXCEPTION("Invalid Engine Name %s", type.c_str());
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


json OutputEngineStore::getRunInfo() {
  auto a = getEngineManager();
  if (a != nullptr) return a->getRunInfo();
  return json::object();
}
std::string OutputEngineStore::getFilePath() {
  auto a = getEngineManager();
  if (a != nullptr) return a->getFilePath();
  return "";
}


json OutputEngineStore::schema(json& packageJson) {
  nlohmann::json m = R"({"type":"object"})"_json;
  nlohmann::json properties = json::object();

  for (auto& it : registeredEngines) {
    properties[it.first] = json::parse((*registeredEngineSchema[it.first])());  // todo
  }
  m["properties"] = properties;
  m["minProperties"] = 1;
  m["maxProperties"] = 1;
  m["additionalProperties"] = false;
  return m;
}

void OutputEngineStore::registerEngine(std::string name, engine_register_ptr engine_ptr, VnV::engine_schema_ptr s) {
  registeredEngines.insert(std::make_pair(name, engine_ptr));
  registeredEngineSchema.insert(std::make_pair(name, s));
}



OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw INJECTION_BUG_REPORT_("Engine Not Initialized Error");
}

BaseStoreInstance(OutputEngineStore)
