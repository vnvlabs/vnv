
/** @file ActionStore.cpp **/

#include "base/stores/ActionStore.h"

#include <dlfcn.h>

#include <iostream>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/Logging.h"
#include "shared/constants.h"

using namespace VnV::Constants;
using namespace VnV;

ActionStore::ActionStore() {}

nlohmann::json ActionStore::schema(json& packageJson) {
  nlohmann::json m = R"({"type":"object"})"_json;
  nlohmann::json props = json::object();
  for (auto& it : action_factory) {
    std::vector<std::string> a;
    StringUtils::StringSplit(it.first, ":", a);
    props[it.first] = it.second.first;
    props[it.first]["description"] = packageJson[a[0]]["Actions"][a[1]]["docs"]["description"];
  }
  m["properties"] = props;
  m["additionalProperties"] = false;
  return m;
}

ActionStore& ActionStore::instance() { return RunTime::instance().store<ActionStore>(); }


void ActionStore::initialize(ICommunicator_ptr world) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::init);
        action->setComm(world);
        action->initialize();
        action->popComm();
        getEngine()->actionEndedCallBack(ActionStage::init);
      } catch (std::exception& e) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage Initialization", action->getPackage().c_str(),
                  action->getName().c_str());
      }
    }
  }

 void ActionStore::injectionPointStart(ICommunicator_ptr comm, std::string packageName, std::string id) {
    for (auto action : actions) {
      if (action->implements_injectionPointStart) {
        try {
          getEngine()->actionStartedCallBack(comm, action->getPackage(), action->getName(), ActionStage::start);
          action->setComm(comm);
          action->injectionPointStart(packageName, id);
          getEngine()->actionEndedCallBack(ActionStage::start);
        } catch (std::exception& e) {
          VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP %s:%s", action->getPackage().c_str(),
                    action->getName().c_str(), packageName.c_str(), id.c_str());
        }
      }
    }
  };

   void ActionStore::injectionPointIter(std::string id) {
    for (auto action : actions) {
      if (action->implements_injectionPointIter) {
        try {
          getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(),
                                             ActionStage::iter);
          action->injectionPointIteration(id);
          getEngine()->actionEndedCallBack(ActionStage::iter);
        } catch (std::exception& e) {
          VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP iteration %s", action->getPackage().c_str(),
                    action->getName().c_str(), id.c_str());
        }
      }
    }
  };

   void ActionStore::injectionPointEnd() {
    for (auto action : actions) {
      if (action->implements_injectionPointEnd) {
        try {
          getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(),
                                             ActionStage::end);
          action->injectionPointEnd();
          action->popComm();
          getEngine()->actionEndedCallBack(ActionStage::end);
        } catch (std::exception& e) {
          VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP end  ", action->getPackage().c_str(),
                    action->getName().c_str());
        }
      }
    }
  }

   void ActionStore::finalize(ICommunicator_ptr world) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::final);
        action->setComm(world);
        action->finalize();
        action->popComm();
        getEngine()->actionEndedCallBack(ActionStage::final);
      } catch (std::exception& e) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage finalize", action->getPackage().c_str(),
                  action->getName().c_str());
      }
    }
  }


void VnV::registerVnVAction(std::string packageName, std::string name, std::string schema, action_ptr m) {
  try {
    json j = json::parse(schema);
    ActionStore::instance().registerAction(packageName, name, j, m);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Could not register action %s:%s --> %s", packageName.c_str(), name.c_str(), e.what());
  }
}

