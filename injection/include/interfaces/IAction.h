
#ifndef VV_IACTION_H
#define VV_IACTION_H

#include <memory>
#include <stdexcept>
#include <vector>

#include "interfaces/ICommunicator.h"
#include "base/parser/JsonSchema.h"
#include "base/ActionType.h"
#include "interfaces/argType.h"
namespace VnV {

class IAction {
 protected:
  ICommunicator_ptr comm;
  json config = nlohmann::json::object();
  json schema = getDefaultOptionsSchema();
 
 public:
  /**
   * @brief IActioner
   */
  IAction(){};

  /**
   * @brief ~IActioner
   */
  virtual ~IAction(){};

  /**
   * @brief run the action
   */
  virtual void run(ActionType type) = 0;

  bool setConfig(const json& conf) { 
    bool r = VnV::validateSchema(conf, schema, false);
    config = conf;
    return r;
  }
  void setSchema(std::string schemaStr) {
    schema = json::parse(schemaStr);
  }

  void setComm(ICommunicator_ptr ptr) { comm = ptr; }
  ICommunicator_ptr getComm() { return comm; }

};

typedef IAction* action_ptr();
void registerAction(std::string packageName, std::string name, VnV::action_ptr ptr);

template <typename Runner> class Actioner_T : public IAction {
 public:
  std::shared_ptr<Runner> runner;
  Actioner_T(std::string schema) { runner.reset(new Runner()); setSchema(schema); }
};

}  // namespace VnV


#define INJECTION_ACTION_R(PNAME, name, Runner, Schema)              \
  namespace VnV {                                                     \
  namespace PNAME {                                                   \
  namespace Actions {                                               \
  class name : public VnV::Actioner_T<VnV_Arg_Type(Runner)> {       \
   public:                                                            \
    name() : VnV::Actioner_T<VnV_Arg_Type(Runner)>(Schema) {             \
                                                                    \
    }                                                                \
    virtual void run(ActionType type) override;                       \
  };                                                                  \
  IAction* declare_##name() { return new name(); }                  \
  void register_##name() {                                            \
    registerAction(VNV_STR(PNAME), #name, declare_##name); \
  }                                                                   \
  }                                                                   \
  }                                                                   \
  }                                                                   \
  void VnV::PNAME::Actions::name::run(ActionType type)

#define INJECTION_ACTION(PNAME, name) \
  INJECTION_ACTION_R(PNAME, name, int, "{\"type\": \"object\"}")


#define INJECTION_ACTION_S(PNAME, name, Schema) \
  INJECTION_ACTION_R(PNAME, name, int, Schema)


#define INJECTION_ACTION_RAW(PNAME, name, cls)       \
  namespace VnV {                                             \
  namespace PNAME {                                           \
  namespace Actions {                                       \
  IAction* declare_##name() { return new cls(); }           \
  void register_##name() {                                    \
    registerAction(#PNAME, #name, declare_##name); \
  }                                                           \
  }                                                           \
  }                                                           \
  }

#define DECLAREACTION(PNAME, name) \
  namespace VnV {                    \
  namespace PNAME {                  \
  namespace Actions {              \
  void register_##name();            \
  }                                  \
  }                                  \
  }

#define REGISTERACTION(PNAME, name) VnV::PNAME::Actions::register_##name();

#endif  // IACTIONS_H
