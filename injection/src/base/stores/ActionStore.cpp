﻿
/** @file ActionStore.cpp **/

#include "base/stores/ActionStore.h"

#include <dlfcn.h>

#include <iostream>

#include "base/stores/CommunicationStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/Utilities.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

ActionStore::ActionStore() {}

ActionStore& ActionStore::getActionStore() {
  static ActionStore store;
  return store;
}

void ActionStore::addAction(std::string packageName, std::string name,  action_ptr m) {
  if (action_factory.find(packageName) == action_factory.end()) {
    action_factory[packageName] = {};
  }
  action_factory[packageName][name] = m;
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

void ActionStore::runAction(Communication::ICommunicator_ptr comm,
                            std::string packageName, std::string name,
                            const json& config,
                            IAction* action) {
  
  
  action->setComm(comm);
  if (action->setConfig(config)) {
    action->run();
  } else {
    VnV_Warn(VNVPACKAGENAME, "Invalid configuration for action %s:%s", packageName.c_str(), name.c_str());
  }
}

void ActionStore::runAction(Communication::ICommunicator_ptr comm,
                            std::string packageName, std::string name, const json& config) {
  auto it = action_factory.find(packageName);
  if (it != action_factory.end()) {
    auto itt = it->second.find(name);
    if (itt != it->second.end()) {
      runAction(comm, packageName, name, config, itt->second());
    }
  }
}

void ActionStore::runAll(VnV_Comm comm, VnV::ActionInfo info) {
  if (!info.run) {
    return;
  }

  auto c = CommunicationStore::instance().getCommunicator(comm);
  for (auto & it : info.actions) {
    if (it.run) {
       runAction(c,it.package, it.name, it.config);
    }
  }

}

void ActionStore::print() {
  int a = VnV_BeginStage(VNVPACKAGENAME, "Registered Actions");
  for (auto it : action_factory) {
    VnV_Info(VNVPACKAGENAME, "Action Package: %s ", it.first.c_str());
    auto aa = VnV_BeginStage(VNVPACKAGENAME, "Actions");
    for (auto itt : it.second) {
      VnV_Info(VNVPACKAGENAME, "%s", itt.first.c_str());
    }
    VnV_EndStage(VNVPACKAGENAME, aa);
  }
  VnV_EndStage(VNVPACKAGENAME, a);
}


void VnV::registerAction(std::string packageName, std::string name,
                             action_ptr m) {
  ActionStore::getActionStore().addAction(packageName, name, m);
}
