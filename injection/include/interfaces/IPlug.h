#ifndef VNV_PLUG_INTERFACE
#define VNV_PLUG_INTERFACE

#include "interfaces/IIterator.h"

namespace VnV {

class PlugConfig : public IteratorConfig {
 public:
  PlugConfig(std::string package, std::string name, json& userConfig) : IteratorConfig(package, name, userConfig) {}
};

class IPlug : public IIterator {
 public:
  IPlug(TestConfig& config) : IIterator(config){};

  virtual int iterate(ICommunicator_ptr /* comm */, IOutputEngine* /* engine */) {
    throw INJECTION_EXCEPTION("Called iterate on a plug %s:%s", m_config.getPackage().c_str(),
                              m_config.getName().c_str());
  };
  virtual bool plug(ICommunicator_ptr comm, IOutputEngine* engine) = 0;
  bool plug_(ICommunicator_ptr comm, OutputEngineManager* engine) {
    engine->testStartedCallBack(m_config.getPackage(), m_config.getName(), false, uuid);
    bool s = plug(comm, engine->getOutputEngine());
    engine->testFinishedCallBack(true);
    return s;
  }
};

typedef IPlug* (*plug_maker_ptr)(PlugConfig config);

void registerPlug(std::string package, std::string name, std::string schema, VnV::plug_maker_ptr m);

}  // namespace VnV

#define INJECTION_PLUG_RS(PNAME, name, Runner, schema)                                  \
  namespace VnV {                                                                       \
  namespace PNAME {                                                                     \
  namespace Plugs {                                                                     \
  class name : public Test_T<VnV_Arg_Type(Runner), IPlug> {                             \
   public:                                                                              \
    name(PlugConfig& config) : Test_T<VnV_Arg_Type(Runner), IPlug>(config) {}           \
                                                                                        \
    bool plug(ICommunicator_ptr comm, VnV::IOutputEngine* engine) override;             \
  };                                                                                    \
  IPlug* declare_##name(PlugConfig config) { return new name(config); }                 \
  void register_##name() { VnV::registerPlug(#PNAME, #name, schema, &declare_##name); } \
  }                                                                                     \
  }                                                                                     \
  }                                                                                     \
  bool VnV::PNAME::Plugs::name::plug(ICommunicator_ptr comm, VnV::IOutputEngine* engine)

#define DECLAREPLUG(PNAME, name) VNVDECLAREMACRO(Plugs, PNAME, name)
#define REGISTERPLUG(PNAME, name) VNVREGISTERMACRO(Plugs, PNAME, name)
#define INJECTION_PLUG_R(PNAME, name, runner) INJECTION_PLUG_RS(PNAME, name, runner, R"({"type":"object"})")
#define INJECTION_PLUG(PNAME, name) INJECTION_PLUG_R(PNAME, name, int)

#endif
