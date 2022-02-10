
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

void OutputEngineStore::setEngineManager(ICommunicator_ptr world, std::string type, json& config) {
  auto it = registeredEngines.find(type);
  if (it != registeredEngines.end()) {
    try {
      manager.reset((*it->second)());
    } catch (std::exception& e) {
      HTHROW INJECTION_EXCEPTION("Engine init failed:  %s", e.what());
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
    } catch (std::exception &e) {
      HTHROW INJECTION_EXCEPTION_("Invalid Engine Schema");
    }
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

json OutputEngineStore::getRunInfo() {
  auto a = getEngineManager();
  if (a != nullptr) return a->getRunInfo();
  return json::object();
}

json OutputEngineStore::schema() {
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

void OutputEngineStore::registerReader(std::string name, engine_reader_ptr engine_ptr, VnV::engine_schema_ptr s) {
  registeredReaders.insert(std::make_pair(name, engine_ptr));
  registeredReaderSchema.insert(std::make_pair(name, s));
}

std::shared_ptr<Nodes::IRootNode> OutputEngineStore::readFile(std::string filename, std::string engineType,
                                                              json& config, bool async) {
  auto it = registeredReaders.find(engineType);

  if (it != registeredReaders.end()) {
    json schema = json::parse((*registeredReaderSchema[engineType])());
    try {
      if (!schema.empty()) {
        json_validator validator;
        validator.set_root_schema(schema);
        validator.validate(config);
      }
    } catch (std::exception &e) {
      throw INJECTION_EXCEPTION("Invalid Engine Reader Configuration: %s", engineType.c_str());
    }

    return (*it->second)(filename, idCounter, config, async);
  }
  throw INJECTION_EXCEPTION("Invalid Engine Reader %s ", engineType.c_str());
}

OutputEngineManager* OutputEngineStore::getEngineManager() {
  if (manager != nullptr) return manager.get();
  throw INJECTION_BUG_REPORT_("Engine Not Initialized Error");
}

BaseStoreInstance(OutputEngineStore)
