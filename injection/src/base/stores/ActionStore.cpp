
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

nlohmann::json ActionStore::schema(json& packageJson) {
  nlohmann::json m = R"({"type":"object"})"_json;
  nlohmann::json props = json::object();
  for (auto& it : action_factory) {
    
    std::vector<std::string> a;
    StringUtils::StringSplit(it.first,":",a);
    props[it.first] = it.second.first;
    props[it.first]["description"] = packageJson[a[0]]["Actions"][a[1]]["docs"]["description"];
  }
  m["properties"] = props;
  m["additionalProperties"] = false;
  return m;
}

ActionStore& ActionStore::instance() { return RunTime::instance().store<ActionStore>(); }


void VnV::registerVnVAction(std::string packageName, std::string name, std::string schema, action_ptr m) {
  try {
    json j = json::parse(schema);
    ActionStore::instance().registerAction(packageName, name, j, m);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Could not register action %s:%s --> Invalid Schema", packageName.c_str(), name.c_str());
  }
}