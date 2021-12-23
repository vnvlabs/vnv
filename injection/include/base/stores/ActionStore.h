
/**
  @file ActionStore.h
**/
#ifndef VV_ACTIONSTORE_HEADER
#define VV_ACTIONSTORE_HEADER

#include <map>
#include <string>
#include "base/parser/JsonSchema.h"
#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "base/stores/OutputEngineStore.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IAction.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {

class ActionStore : public BaseStore {
  std::map<std::string, std::pair<json, action_ptr>> action_factory;

  std::vector<std::shared_ptr<IAction>> actions;

 public:
  ActionStore();

  OutputEngineManager* getEngine() { return OutputEngineStore::instance().getEngineManager(); }

  virtual void initialize(const ActionInfo& info) {
    if (info.run) {
      for (auto it : info.actions) {
        if (it.run) {
          try {
            addAction(it.package, it.name, it.config);
          } catch (VnVExceptionBase& e) {
            VnV_Error(VNVPACKAGENAME, "Action %s:%s does not exist -- Ignoring ", it.package.c_str(), it.name.c_str());
          }
        }
      }
    }
  }

  virtual void initialize(ICommunicator_ptr world) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::init);
        action->setComm(world);
        action->initialize();
        action->popComm();
        getEngine()->actionEndedCallBack(ActionStage::init);
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage Initialization", action->getPackage().c_str(),
                  action->getName().c_str());
      }
    }
  }

  virtual void injectionPointStart(ICommunicator_ptr comm, std::string packageName, std::string id) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(comm, action->getPackage(), action->getName(), ActionStage::start);
        action->setComm(comm);
        action->injectionPointStart(packageName, id);
        getEngine()->actionEndedCallBack(ActionStage::start);
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP %s:%s", action->getPackage().c_str(),
                  action->getName().c_str(), packageName.c_str(), id.c_str());
      }
    }
  };

  virtual void injectionPointIter(std::string id) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(),
                                           ActionStage::iter);
        action->injectionPointIteration(id);
        getEngine()->actionEndedCallBack(ActionStage::iter);
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP iteration %s", action->getPackage().c_str(),
                  action->getName().c_str(), id.c_str());
      }
    }
  };

  virtual void injectionPointEnd() {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(),
                                           ActionStage::end);
        action->injectionPointEnd();
        action->popComm();
        getEngine()->actionEndedCallBack(ActionStage::end);
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage IP end  ", action->getPackage().c_str(),
                  action->getName().c_str());
      }
    }
  }

  virtual void finalize(ICommunicator_ptr world) {
    for (auto action : actions) {
      try {
        getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::final);
        action->setComm(world);
        action->finalize();
        action->popComm();
        getEngine()->actionEndedCallBack(ActionStage::final);
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Action %s:%s failed at stage finalize", action->getPackage().c_str(),
                  action->getName().c_str());
      }
    }
  }

  void registerAction(std::string packageName, std::string name, const json& schema, action_ptr m) {
    action_factory[packageName + ":" + name] = {schema, m};
  }

  bool registeredAction(std::string packageName, std::string name) {
    return action_factory.find(packageName + ":" + name) == action_factory.end();
    
  }

  void addAction(std::string packageName, std::string name, const json& config) {
    auto it = action_factory.find(packageName + ":" + name);
    if (it != action_factory.end()) {
      try {
        VnV::validateSchema(config, it->second.first, true);
        std::shared_ptr<VnV::IAction> act;
        act.reset((*(it->second.second))(config));
        act->setNameAndPackageAndEngine(packageName, name, getEngine());
        actions.push_back(act);
        return;
      } catch (...) {

      }
    } 
    HTHROW INJECTION_EXCEPTION("Error adding Action", packageName.c_str(), name.c_str());
  }

  nlohmann::json schema();

  static ActionStore& instance();
};

}  // namespace VnV

#endif
