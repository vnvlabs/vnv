#ifndef VNV_ITERATOR_INTERFACE
#define VNV_ITERATOR_INTERFACE
#include "interfaces/ITest.h"

namespace VnV {

class IteratorConfig : public TestConfig {
 public:
  IteratorConfig(std::string package, std::string name, json& userConfig) : TestConfig(package, name, userConfig) {}
};

class IIterator : public ITest {
 public:
  IIterator(VnV::TestConfig& config) : ITest(config){};

  int iterate_(ICommunicator_ptr comm, OutputEngineManager* engine) {
    engine->testStartedCallBack(m_config.getPackage(), m_config.getName(), false, uuid);
    int s = iterate(comm, engine->getOutputEngine());
    engine->testFinishedCallBack(true);
    return s;
  }

  virtual int iterate(ICommunicator_ptr comm, IOutputEngine* engine) = 0;

  virtual TestStatus runTest(ICommunicator_ptr comm, IOutputEngine* engine, InjectionPointType type,
                             std::string stageId) {
    throw VnV::VnVExceptionBase("Called RunTest on an iterator");
  };
};

typedef IIterator* (*iterator_maker_ptr)(IteratorConfig config);

void registerIterator(std::string package, std::string name, std::string schema, VnV::iterator_maker_ptr m);

}  // namespace VnV

#define INJECTION_ITERATOR_RS(PNAME, name, Runner, schema)                                  \
  namespace VnV {                                                                           \
  namespace PNAME {                                                                         \
  namespace Iterators {                                                                     \
  class name : public Test_T<VnV_Arg_Type(Runner), VnV::IIterator> {                        \
   public:                                                                                  \
    name(IteratorConfig& config) : Test_T<VnV_Arg_Type(Runner), VnV::IIterator>(config) {}  \
    int iterate(ICommunicator_ptr comm, IOutputEngine* engine) override;                    \
  };                                                                                        \
  IIterator* declare_##name(IteratorConfig config) { return new name(config); }             \
  void register_##name() { VnV::registerIterator(#PNAME, #name, schema, &declare_##name); } \
  }                                                                                         \
  }                                                                                         \
  }                                                                                         \
  int VnV::PNAME::Iterators::name::iterate(ICommunicator_ptr comm, VnV::IOutputEngine* engine)

#define DECLAREITERATOR(PNAME, name) VNVDECLAREMACRO(Iterators, PNAME, name)
#define REGISTERITERATOR(PNAME, name) VNVREGISTERMACRO(Iterators, PNAME, name)
#define INJECTION_ITERATOR_R(PNAME, name, runner) INJECTION_ITERATOR_RS(PNAME, name, runner, R"({"type":"object"})")
#define INJECTION_ITERATOR(PNAME, name) INJECTION_ITERATOR_R(PNAME, name, int)

#endif