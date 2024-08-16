
/**
  @file ActionStore.h
**/
#ifndef VV_ACTIONSTORE_HEADER
#define VV_ACTIONSTORE_HEADER

#include <map>
#include <string>

#include "common-interfaces/all.h"
#include "base/stores/BaseStore.h"
#include "base/stores/OutputEngineStore.h"
#include "interfaces/IAction.h"

namespace VnV
{

  class ActionStore : public BaseStore
  {

    std::map<std::string, std::pair<json, action_ptr>> action_factory;

    std::vector<std::shared_ptr<IAction>> actions;

  public:
    ActionStore();

    OutputEngineManager *getEngine();

    virtual void initialize(ICommunicator_ptr world);

    virtual void injectionPointStart(ICommunicator_ptr comm, std::string packageName, std::string id);

    virtual void injectionPointIter(std::string id);

    virtual void injectionPointEnd();

    virtual void finalize(ICommunicator_ptr world);

    void registerAction(std::string packageName, std::string name, const json &schema, action_ptr m);

    bool registeredAction(std::string packageName, std::string name);

    void print();

    void addAction(std::string packageName, std::string name, const json &config);

    nlohmann::json schema(json &packageJson);

    static ActionStore &instance();
  };

} // namespace VnV

#endif
