
/** @file ActionStore.cpp **/

#include "base/stores/ActionStore.h"

#include <dlfcn.h>

#include <iostream>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/OutputEngineStore.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

ActionStore::ActionStore() {}


nlohmann::json ActionStore::schema() {
  nlohmann::json m = R"({"type":"object"})"_json;
  nlohmann::json props = json::object();
  for (auto& it : action_factory) {
    props[it.first] = it.second.first;
  }
  m["properties"] = props;
  m["additionalProperties"] = false;
  return m;
}


ActionStore& ActionStore::instance() {
  return RunTime::instance().store<ActionStore>();
}

void VnV::registerAction(std::string packageName, std::string name, std::string schema,
                         action_ptr m) {
  json j = json::parse(schema);
  ActionStore::instance().registerAction(packageName, name, j, m);
}