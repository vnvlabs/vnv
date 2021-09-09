
/** @file ActionStore.cpp **/

#include "base/stores/ActionStore.h"

#include <dlfcn.h>

#include <iostream>

#include "base/stores/CommunicationStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/Utilities.h"
#include "c-interfaces/Logging.h"
#include "base/Runtime.h"

using namespace VnV;

ActionStore::ActionStore() {}

void ActionStore::addAction(std::string packageName, std::string name,  action_ptr m) {
  if (action_factory.find(packageName) == action_factory.end()) {
    action_factory[packageName] = {};
  }
  action_factory[packageName][name] = m;
}

nlohmann::json ActionStore::schema() {
    
    nlohmann::json m = R"({"type":"object"})"_json;
    nlohmann::json props = json::object();  
    for (auto &it :  action_factory) {
      
      nlohmann::json m1 = R"({"type":"object"})"_json;
      nlohmann::json props1 = json::object();
      
      for (auto &itt : it.second) {
         props1[itt.first] = R"({"type" : "string" , "enum" : ["off", "start","end","both"] })"_json;       
      }
      m1["properties"] = props1;
      props[it.first] = m1;

    }
    m["properties"] = props;
    m["additionalProperties"] = false;
    return m;
}


IAction* ActionStore::getAction(std::string packageName,
                                        std::string name) {
  auto it = action_factory.find(packageName);
  if (it != action_factory.end()) {
    auto itt = it->second.find(name);
    if (itt != it->second.end()) {
      return itt->second();
    }
  }
  return nullptr;
}



void ActionStore::runAction(ICommunicator_ptr comm,
                            std::string packageName, std::string name,
                            const json& config,
                            IAction* action, ActionType& type) {
  
  
  action->setComm(comm);
  if (action->setConfig(config)) {
    action->run(type);
  } else {
    VnV_Warn(VNVPACKAGENAME, "Invalid configuration for action %s:%s", packageName.c_str(), name.c_str());
  }
}

void ActionStore::runAction(ICommunicator_ptr comm,
                            std::string packageName, std::string name, const json& config, ActionType& type) {
  auto it = action_factory.find(packageName);
  if (it != action_factory.end()) {
    auto itt = it->second.find(name);
    if (itt != it->second.end()) {
      runAction(comm, packageName, name, config, (*itt->second)(),type);
    }
  }
}

void ActionStore::runAll(VnV_Comm comm, VnV::ActionInfo info, ActionType& type) {
  if (!info.run) {
    return;
  }

  auto c = CommunicationStore::instance().getCommunicator(comm);
  for (auto & it : info.actions) {
    if ( type.equals(it.run) ) {
       runAction(c,it.package, it.name, it.config,type);
    }
  }

}

void ActionStore::print() {
  for (auto it : action_factory) {
    VnV_Info(VNVPACKAGENAME, "Action Package: %s ", it.first.c_str());
    for (auto itt : it.second) {
      VnV_Info(VNVPACKAGENAME, "%s", itt.first.c_str());
    }
  }
}

ActionStore& ActionStore::instance() {
  return RunTime::instance().store<ActionStore>();
}

void VnV::registerAction(std::string packageName, std::string name,
                             action_ptr m) {
  ActionStore::instance().addAction(packageName, name, m);
}
