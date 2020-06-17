﻿#ifndef ITEST_H
#define ITEST_H

#include <map>
#include <string>

#include "base/TransformStore.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/Logging.h"
#include "interfaces/IOutputEngine.h"
#include "interfaces/ITransform.h"
#include "interfaces/argType.h"
#include "json-schema.hpp"
//#include "c-interfaces/Logging.h"
using nlohmann::json;
using nlohmann::json_schema::json_validator;
/**
 * @brief The TestConfig class
 */
namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };

class VnVParameter {
  void* ptr;
  std::string rtti;
  std::string type;
  bool hasRtti = true;

 public:
  VnVParameter() { ptr = nullptr; }

  VnVParameter(void* obj, std::string type_) {
    rtti = "";
    type = type_;
    ptr = obj;
    hasRtti = false;
  }

  VnVParameter(const VnVParameter& copy) {
    ptr = copy.getRawPtr();
    rtti = copy.getRtti();
    type = copy.getType();
    hasRtti = copy.hasRtti;
  }

  VnVParameter(void* obj, std::string type_, std::string rtti_) {
    rtti = rtti_;
    type = type_;
    ptr = obj;
    hasRtti = (rtti.size() > 0);
  }

  void setType(std::string type) { this->type = type; }

  void setRtti(std::string rtti) { this->rtti = rtti; }

  void* getRawPtr() const { return ptr; }

  std::string getType() const { return type; }

  std::string getRtti() const { return rtti; }

  template <typename T> T* getPtr(std::string type, bool checkRtti) const {
    // If we have RTTI and it matches, then that is good enough.
    if (hasRtti && checkRtti) {
      T* tempPtr = static_cast<T*>(getRawPtr());
      std::string typeId = typeid(tempPtr).name();
      if (typeId.compare(rtti) != 0) {
        VnV_Warn("Unmatched RTTI %s: %s", typeId.c_str(), rtti.c_str());
      } else {
        return tempPtr;
      }
    }
    if (!type.empty() && getType().compare(type) != 0) {
      throw VnVExceptionBase("type information incorrect");
    }
    return static_cast<T*>(getRawPtr());
  }

  template <typename T> T& getRef(std::string type, bool checkRtti) const {
    return *getPtr<T>(type, checkRtti);
  }
};
typedef std::map<std::string, VnVParameter> VnVParameterSet;

class TestConfig {
 private:
  std::map<std::string, std::shared_ptr<Transformer> > transformers;
  VnVParameterSet parameters;
  std::map<std::string, std::string> testParameters;
  std::string testName;
  std::string package;
  json testConfigJson;

 public:
  TestConfig(std::string package, std::string name, json& usersConfig,
             std::map<std::string, std::string>& params);

  bool isRequired(std::string parmaeterName) const;
  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;

  bool preLoadParameterSet(std::map<std::string, std::string>& parameters);

  /**
   * @brief setName
   * @param name
   */
  void setName(std::string name);

  const std::map<std::string, VnVParameter>& getParameterMap() const;

  void setParameterMap(std::map<std::string, VnVParameter>& args);

  /**
   * @brief getName
   * @return
   */
  std::string getName() const;
  std::string getPackage() const;
  /**
   * @brief print out configuration information.
   */
  void print();

  void setUnknownInjectionPoint();
};

/**
 * @brief The ITest class
 */
class ITest {
 public:
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
  TestStatus _runTest(VnV_Comm comm, OutputEngineManager* engine,
                      InjectionPointType type, std::string stageId);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(VnV_Comm comm, IOutputEngine* engine,
                             InjectionPointType type, std::string stageId) = 0;

  /**
   * @brief getConfigurationJson
   * @return
   */
  const json& getConfigurationJson() const;

  template <typename T> T* getPtr(std::string name, std::string type) const {
    StringUtils::squash(type);
    auto it = m_config.getParameterMap().find(name);
    if (it != m_config.getParameterMap().end()) {
      return it->second.getPtr<T>(type, true);
    }
    return nullptr;
  }

  template <typename T>
  T& getReference(std::string name, std::string type) const {
    StringUtils::squash(type);

    auto it = m_config.getParameterMap().find(name);
    if (it != m_config.getParameterMap().end()) {
      return it->second.getRef<T>(type, true);
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }

 private:
  const TestConfig m_config;

  /**
   * @brief carefull_cast
   * @tparam T class type
   * @param stage Test Stage
   * @param parameterName The name of the parameter to convert
   * @param parameters The Map of parameters passed to the Test by the injection
   * point.
   */
};

// Search in s for a VnVParameter named "name". Convert the raw ptr to class T
// with checking.
#define GetRef(a, name, T) T& a = getReference<T>(name, #T);

typedef ITest* maker_ptr(TestConfig config);

void registerTest(std::string package, std::string name, VnV::maker_ptr m,
                  std::map<std::string, std::string> parameters);
void registerTestSchema(std::string package, std::string name, std::string schema);

/// Macros to make it easier to define one.

// Process a list of comma seperated variadic args into a map.

#define VNVVARSTR(...) StringUtils::variadicProcess(#__VA_ARGS__)

template <typename Runner >
class Test_T : public ITest {
 public:
  std::map<std::string, std::string> parameters;
  std::shared_ptr<Runner> runner;

  Test_T(TestConfig& config, const char* params) : ITest(config) {
    runner.reset(new Runner());
    parameters = StringUtils::variadicProcess(params);
  }
  template <typename T> T& get(std::string param) {
    auto it = parameters.find(param);
    if (it != parameters.end()) {
      return *getPtr<T>(param, it->second);
    }
    throw VnV::VnVExceptionBase("Parameter does not exist");
  }

  static void registerTest(std::string name, maker_ptr* maker,
                           const char* params) {
    VnV::registerTest(PACKAGENAME_S, name, maker,
                      StringUtils::variadicProcess(params));
  }
};

}  // namespace VnV

#define INJECTION_TEST_RS(name, Runner, schema, ...)                            \
  namespace VnV {                                                              \
  namespace PACKAGENAME {                                                      \
  namespace Tests {                                                            \
  class name : public Test_T<VnV_Arg_Type(Runner)> {                           \
   public:                                                                     \
    name(TestConfig& config)                                                   \
        : Test_T<VnV_Arg_Type(Runner)>(config, #__VA_ARGS__ ) {}                \
    TestStatus runTest(VnV_Comm comm, IOutputEngine* engine,                   \
                       InjectionPointType type, std::string stageId);          \
  };                                                                           \
  ITest* declare_##name(TestConfig config) { return new name(config); }        \
  void register_##name() {                                                     \
     name::registerTest(#name, declare_##name, #__VA_ARGS__);                   \
     VnV::registerTestSchema(PACKAGENAME_S,#name, schema);                                               \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  VnV::TestStatus VnV::PACKAGENAME::Tests::name::runTest(                      \
      VnV_Comm comm, VnV::IOutputEngine* engine, VnV::InjectionPointType type, \
      std::string stageId)


#define INJECTION_TEST_R(name, runner,...) INJECTION_TEST_RS(name,runner,R"({"type":"object"})",__VA_ARGS__)
#define INJECTION_TEST(name, ...) INJECTION_TEST_R(name, int, __VA_ARGS__)

#define DECLARETEST(name) \
  namespace VnV {         \
  namespace PACKAGENAME { \
  namespace Tests {       \
  void register_##name(); \
  }                       \
  }                       \
  }
#define REGISTERTEST(name) VnV::PACKAGENAME::Tests::register_##name();

#endif  // ITEST_H
