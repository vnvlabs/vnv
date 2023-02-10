
#ifndef VV_IACTION_H
#define VV_IACTION_H

#include <memory>
#include <stack>
#include <stdexcept>
#include <vector>

#include "interfaces/ActionType.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"
#include "interfaces/argType.h"
#include "json-schema.hpp"
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

 protected:
  bool implements_injectionPointStart = false;
  bool implements_injectionPointIter = false;
  bool implements_injectionPointEnd = false;

 public:
  virtual void implements_injection_point(bool yes) {
    implements_injectionPointStart = yes;
    implements_injectionPointIter = yes;
    implements_injectionPointEnd = yes;
  }

  virtual ICommunicator_ptr getComm() { return commStack.top(); }

  virtual IOutputEngine* getEngine() { return engine; }

  virtual void initialize(){};

  virtual void injectionPointStart(std::string /**packageName**/, std::string /**id**/) {
    throw INJECTION_EXCEPTION_("Implements Initialize Called");
  };

  virtual void injectionPointIteration(std::string /**stageId**/) {
    throw INJECTION_EXCEPTION_("Implements Initialize Called");
  };

  virtual void injectionPointEnd() { throw INJECTION_EXCEPTION_("Implements Initialize Called"); };

  virtual void finalize() {}

  virtual ~IAction() = default;
  IAction() = default;
};

typedef IAction* (*action_ptr)(const nlohmann::json&);

// Added VnV because Moose has a macro called registerAction :)
void registerVnVAction(std::string packageName, std::string name, std::string schema, VnV::action_ptr ptr);

}  // namespace VnV

#define INJECTION_ACTION(PNAME, name, schema)                                                       \
  namespace VnV {                                                                                   \
  namespace PNAME {                                                                                 \
  namespace Actions {                                                                               \
  IAction* declare_##name(const json& config);                                                      \
  void register_##name() { VnV::registerVnVAction(VNV_STR(PNAME), #name, schema, declare_##name); } \
  }                                                                                                 \
  }                                                                                                 \
  }                                                                                                 \
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
