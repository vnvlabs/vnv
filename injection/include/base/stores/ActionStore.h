
/**
  @file ActionStore.h
**/
#ifndef VV_ACTIONSTORE_HEADER
#define VV_ACTIONSTORE_HEADER

#include <map>
#include <string>

#include "base/parser/JsonParser.h"
#include "base/parser/JsonSchema.h"
#include "base/stores/BaseStore.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/Communication.h"
#include "common-interfaces/PackageName.h"
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
          } catch (...) {
            VnV_Error(VNVPACKAGENAME, "Action %s:%s does not exist -- Ignoring ", it.package.c_str(), it.name.c_str());
          }
        }
      }
    }
  }

  virtual void initialize(ICommunicator_ptr world); 

  virtual void injectionPointStart(ICommunicator_ptr comm, std::string packageName, std::string id);

  virtual void injectionPointIter(std::string id) ;

  virtual void injectionPointEnd() ;

  virtual void finalize(ICommunicator_ptr world) ;

  void registerAction(std::string packageName, std::string name, const json& schema, action_ptr m) {
    action_factory[packageName + ":" + name] = {schema, m};
  }

  bool registeredAction(std::string packageName, std::string name) {
    return action_factory.find(packageName + ":" + name) != action_factory.end();
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
      } catch (std::exception& e) {
      }
    }
    throw "Error adding Action";
  }

  nlohmann::json schema(json& packageJson);

  static ActionStore& instance();
};

}  // namespace VnV

#endif
