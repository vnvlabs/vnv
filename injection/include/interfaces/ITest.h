﻿#ifndef ITEST_H
#define ITEST_H

#include <map>
#include <regex>
#include <string>

#include "base/FunctionSigniture.h"
#include "base/InjectionPointConfig.h"
#include "common-interfaces/all.h"
#include "interfaces/IOutputEngine.h"
#include "interfaces/helpers/argType.h"
#include "validate/json-schema.hpp"

using nlohmann::json;
/**
 * @brief The TestConfig class
 */
namespace VnV {

/**
 * @brief The ITest class
 */
class ITest {
 public:
  static long uid;
  long uuid;

  /**
   * @brief ITest
   */
  ITest(TestConfig& config);

  /**
   * @brief ~ITest
   */
  virtual ~ITest();

  /**
   * @brief _runTest
   * @param engine
   * @param stageVal
   * @param params
   * @return

   */
  TestStatus _runTest(ICommunicator_ptr comm, OutputEngineManager* engine, InjectionPointType type,
                      std::string stageId);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(ICommunicator_ptr comm, IOutputEngine* engine, InjectionPointType type,
                             std::string stageId) = 0;

  /**
   * @brief getConfigurationJson
   * @return
   */
  const json& getConfigurationJson() const;

  template <typename T> T* getInputPtr(std::string name) { return getPtr<T>(name,  true); }

  template <typename T> T* getOutputPtr(std::string name) { return getPtr<T>(name, false); }

  template <typename T> T& getInputRef(std::string name) { return getRef<T>(name, true); }

  template <typename T> T& getOutputRef(std::string name) { return getRef<T>(name, false); }

  template <typename T> T* getPtr(std::string name) { return getPtr<T>(name, true); }

  template <typename T> T& getRef(std::string name) { return getRef<T>(name, true); }

  template <typename T> T& getRef(std::string name, bool input) {
    return *getPtr<T>(name, input);
  }

  template <typename T> T* getPtr(std::string name, bool input) {
    return m_config.getParameterMap().getPtr<T>(name, input);
  }

 protected:
  TestConfig m_config;
};

// Little shortcut macros for cases where the type name is the type.
#define GetRef(name, T) getRef<T>(name)
#define GetPtr(name, T) getPtr<T>(name)


typedef ITest* (*maker_ptr)(TestConfig config);
void registerTest(std::string package, std::string name, std::string schema, VnV::maker_ptr m);

// Search in s for a VnVParameter named "name". Convert the raw ptr to class T
// with checking.

template <typename Runner, typename Type> class Test_T : public Type {
 public:
  std::shared_ptr<Runner> runner;

  Test_T(TestConfig& config) : Type(config) { runner.reset(new Runner()); }
};

}  // namespace VnV

 
#define VNVREGISTERMACRO(Type, PNAME, name)

#define INJECTION_TEST_RS(PNAME, name, Runner, schema)                                                               \
  namespace VnV {                                                                                                    \
  namespace PNAME {                                                                                                  \
  namespace Tests {                                                                                                  \
  class name : public Test_T<VnV_Arg_Type(Runner), ITest> {                                                          \
   public:                                                                                                           \
    name(TestConfig& config) : Test_T<VnV_Arg_Type(Runner), ITest>(config) {}                                        \
    TestStatus runTest(ICommunicator_ptr comm, IOutputEngine* engine, InjectionPointType type, std::string stageId); \
  };                                                                                                                 \
  ITest* declare_##name(TestConfig config) { return new name(config); }                                              \
  void register_##name() { VnV::registerTest(VNV_STR(PNAME), #name, schema, &declare_##name); }                      \
  }                                                                                                                  \
  }                                                                                                                  \
  }                                                                                                                  \
  VnV::TestStatus VnV::PNAME::Tests::name::runTest(ICommunicator_ptr comm, VnV::IOutputEngine* engine,               \
                                                   VnV::InjectionPointType type, std::string stageId)


#define INJECTION_TEST_R(PNAME, name, runner) INJECTION_TEST_RS(PNAME, name, runner, R"({"type":"object"})")

#define INJECTION_TEST(PNAME, name) INJECTION_TEST_R(PNAME, name, int)

#define DECLARETEST(PNAME, name) \
 namespace VnV {                          \
  namespace PNAME {                        \
    namespace Tests {                         \
      void register_##name();                  \
    }                                        \
  }                                        \
}

#define REGISTERTEST(PNAME, name) VnV::PNAME::Tests::register_##name();


#endif  // ITEST_H
