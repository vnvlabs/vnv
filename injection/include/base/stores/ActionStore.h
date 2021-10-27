
/**
  @file ActionStore.h
**/
#ifndef VV_ACTIONSTORE_HEADER
#define VV_ACTIONSTORE_HEADER

#include <map>
#include <string>

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
          addAction(it.package, it.name, it.config);
        }
      }
    }
  }

  virtual void initialize(ICommunicator_ptr world) {
    for (auto action : actions) {
      getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::init);
      action->setComm(world);
      action->initialize();
      action->popComm();
      getEngine()->actionEndedCallBack(ActionStage::init);
    }
  }

  virtual void injectionPointStart(ICommunicator_ptr comm, std::string packageName, std::string id) {
    for (auto action : actions) {
      getEngine()->actionStartedCallBack(comm, action->getPackage(), action->getName(), ActionStage::start);
      action->setComm(comm);
      action->injectionPointStart(packageName, id);
      getEngine()->actionEndedCallBack(ActionStage::start);
    }
  };

  virtual void injectionPointIter(std::string id) {
    for (auto action : actions) {
      getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(), ActionStage::iter);
      action->injectionPointIteration(id);
      getEngine()->actionEndedCallBack(ActionStage::iter);
    }
  };

  virtual void injectionPointEnd() {
    for (auto action : actions) {
      getEngine()->actionStartedCallBack(action->getComm(), action->getPackage(), action->getName(), ActionStage::end);
      action->injectionPointEnd();
      action->popComm();
      getEngine()->actionEndedCallBack(ActionStage::end);
    }
  }

  virtual void finalize(ICommunicator_ptr world) {
    for (auto action : actions) {
      getEngine()->actionStartedCallBack(world, action->getPackage(), action->getName(), ActionStage::final);
      action->setComm(world);
      action->finalize();
      action->popComm();
      getEngine()->actionEndedCallBack(ActionStage::final);
    }
  }

  void registerAction(std::string packageName, std::string name, const json& schema, action_ptr m) {
    action_factory[packageName + ":" + name] = {schema, m};
  }

  void addAction(std::string packageName, std::string name, const json& config) {
    auto it = action_factory.find(packageName + ":" + name);
    if (it != action_factory.end()) {
      VnV::validateSchema(config, it->second.first, true);
      std::shared_ptr<VnV::IAction> act;
      act.reset((*(it->second.second))(config));
      act->setNameAndPackageAndEngine(packageName, name, getEngine());
      actions.push_back(act);
    } else {
      throw VnV::VnVExceptionBase("No action with that name and package exists (%s:%s)", packageName.c_str(),
                                  name.c_str());
    }
  }

  nlohmann::json schema();

  static ActionStore& instance();
};

}  // namespace VnV

#endif
