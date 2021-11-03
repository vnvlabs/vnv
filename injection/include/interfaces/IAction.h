
#ifndef VV_IACTION_H
#define VV_IACTION_H

#include <memory>
#include <stack>
#include <stdexcept>
#include <vector>

#include "base/ActionType.h"
#include "base/parser/JsonSchema.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"
#include "interfaces/argType.h"
namespace VnV {

class ActionStore;

class IAction {
  friend class ActionStore;
  std::string name, package;
  IOutputEngine* engine;
  void setNameAndPackageAndEngine(std::string package, std::string name, IOutputEngine* engine) {
    this->name = name;
    this->package = package;
    this->engine = engine;
  }

  std::string getName() { return name; }

  std::string getPackage() { return package; }

  std::stack<ICommunicator_ptr> commStack;

  void setComm(ICommunicator_ptr ptr) { commStack.push(ptr); }
  void popComm() { commStack.pop(); }

 public:
  virtual ICommunicator_ptr getComm() { return commStack.top(); }

  virtual IOutputEngine* getEngine() { return engine; }

  virtual void initialize(){};

  virtual void injectionPointStart(std::string packageName, std::string id){};

  virtual void injectionPointIteration(std::string stageId){};

  virtual void injectionPointEnd(){};

  virtual void finalize() {}

  virtual ~IAction() = default;
  IAction() = default;
};

typedef IAction* (*action_ptr)(const nlohmann::json&);

void registerAction(std::string packageName, std::string name, std::string schema, VnV::action_ptr ptr);

}  // namespace VnV

#define INJECTION_ACTION(PNAME, name, schema)                          \
  namespace VnV {                                                           \
  namespace PNAME {                                                         \
  namespace Actions {                                                       \
  IAction* declare_##name(const json& config);                            \
  void register_##name() { registerAction(VNV_STR(PNAME), #name, schema,  declare_##name); } \
  }                                                                         \
 }                                                                          \
}\
VnV::IAction* VnV::PNAME::Actions::declare_##name(const nlohmann::json& config) 

#define DECLAREACTION(PNAME, name) \
  namespace VnV {                  \
  namespace PNAME {                \
  namespace Actions {              \
  void register_##name();          \
  }                                \
  }                                \
  }

#define REGISTERACTION(PNAME, name) VnV::PNAME::Actions::register_##name();

#endif  // IACTIONS_H
