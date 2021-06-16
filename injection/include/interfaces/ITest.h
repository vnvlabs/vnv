#ifndef ITEST_H
#define ITEST_H

#include <map>
#include <string>

#include "base/stores/TransformStore.h"
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
  bool input = true;

 public:
  VnVParameter() { ptr = nullptr; }

  VnVParameter(void* obj, std::string type_, bool input_) {
    rtti = "";
    type = type_;
    ptr = obj;
    hasRtti = false;
    input = input_;
  }

  VnVParameter(const VnVParameter& copy) {
    ptr = copy.getRawPtr();
    rtti = copy.getRtti();
    type = copy.getType();
    hasRtti = copy.hasRtti;
    input = copy.isInput();
  }

  VnVParameter(void* obj, std::string type_, std::string rtti_, bool input_) {
    rtti = rtti_;
    type = type_;
    ptr = obj;
    hasRtti = (rtti.size() > 0);
    input = input_;
  }

  void setType(std::string type) { this->type = type; }

  void setRtti(std::string rtti) { this->rtti = rtti; }

  void setInput(bool input) {this->input = input;}

  void* getRawPtr() const { return ptr; }

  std::string getType() const { return type; }

  std::string getRtti() const { return rtti; }

  bool isInput() const {return input;};

  template <typename T> T& getByRtti() {
    if (hasRtti) {
      T* tempPtr = static_cast<T*>(getRawPtr());
      std::string typeId = typeid(tempPtr).name();
      if (typeId.compare(getRtti()) == 0) {
        return *tempPtr;
      }
    }
    throw VnVExceptionBase("Invalid Parameter conversion in test");
  }


  template <typename T> T& get_rtti_or_nothing() {
    if (hasRtti) {
      return getByRtti<T>();
    } else {
        T* tempPtr = static_cast<T*>(getRawPtr());
        return *tempPtr;
    }
  }
  template <typename T> T* getByRtti_Ptr() {
    if (hasRtti) {
      T* tempPtr = static_cast<T*>(getRawPtr());
      std::string typeId = typeid(tempPtr).name();
      if (typeId.compare(getRtti()) == 0) {
        return tempPtr;
      }
    }
    throw VnVExceptionBase("Invalid Parameter conversion in test");
  }


  template <typename T> T* getByType_Ptr(std::string type) {
    StringUtils::squash(type);
    if (!type.empty() && getType().compare(type) != 0) {
      throw VnVExceptionBase("type information incorrect");
    }
    return static_cast<T*>(getRawPtr());
  }


  template <typename T>  T& getByType(std::string type) {
    StringUtils::squash(type);
    if (!type.empty() && getType().compare(type) != 0) {
      throw VnVExceptionBase("type information incorrect");
    }
    return *(static_cast<T*>(getRawPtr()));
  }

  template <typename T> T& getByRttiOrType(std::string type)  {
    try {
      return getByRtti<T>();
    } catch (...) {
      return getByType<T>(type);
    }
  }

  template <typename T> T* getByRttiOrType_Ptr(std::string type) {
    try {
      return getByRtti_Ptr<T>();
    } catch (...) {
      return getByType_Ptr<T>(type);
    }
  }

};
typedef std::map<std::string, VnVParameter> VnVParameterSet;


class TestConfig {
 protected:
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

  VnVParameterSet& getParameterMap() ;


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

typedef std::function<void(
    VnV_Comm comm, std::map<std::string, VnV::VnVParameter>& ntv,
    VnV::OutputEngineManager* engine, VnV::InjectionPointType type, std::string stageId)>
    DataCallback;

void defaultCallBack(VnV_Comm comm, std::map<std::string, VnVParameter>& ntv,
                     IOutputEngine* engine, InjectionPointType type,
                     std::string stageId);

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
   * @return          ${CMAKE_CURRENT_LIST_DIR}/TestStore.cpp

   */
  TestStatus _runTest(ICommunicator_ptr comm, OutputEngineManager* engine,
                      InjectionPointType type, std::string stageId);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(ICommunicator_ptr comm, IOutputEngine* engine,
                             InjectionPointType type, std::string stageId) = 0;




  /**
   * @brief getConfigurationJson
   * @return
   */
  const json& getConfigurationJson() const;

  template <typename T>
  T& getReference(std::string name, std::string type) {
    StringUtils::squash(type);
    auto it = m_config.getParameterMap().find(name);
    if (it != m_config.getParameterMap().end()) {
      return it->second.getByRttiOrType<T>(type);
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }

  template <typename T>
  T& getReference(std::string name) {
    auto it = m_config.getParameterMap().find(name);
    if (it != m_config.getParameterMap().end()) {
      return it->second.get_rtti_or_nothing<T>();
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }


  template <typename T>
  T* getInputParameter(std::string name, std::string type)  {
       return getPtr<T>(name, type, true);
  }

  template <typename T>
  T* getOutputParameter(std::string name, std::string type) {
      return getPtr<T>(name, type, false);
  }


 protected:

  TestConfig m_config;


  template <typename T>
  T* getPtr(std::string name, std::string type, bool input)  {
    StringUtils::squash(type);
    VnVParameterSet& map = m_config.getParameterMap();
    auto it = map.find(name);
    if (it != map.end()) {
      if (it->second.isInput() != input) {
          if (input) {
             throw VnVExceptionBase("Requested an input parameter but got an output parameter");
          }
          throw VnVExceptionBase("Requested an output parameter but got an input parameter");
      }
      return it->second.getByRttiOrType_Ptr<T>(type);
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }

  /**
   * @brief carefull_cast
   * @tparam T class type
   * @param stage Test Stage
   * @param parameterName The name of the parameter to convert
   * @param parameters The Map of parameters passed to the Test by the injection
   * point.
   */
};

typedef ITest* maker_ptr(TestConfig config);
void registerTest(std::string package, std::string name,std::string schema, VnV::maker_ptr m,
                  std::map<std::string, std::string> parameters);





// Search in s for a VnVParameter named "name". Convert the raw ptr to class T
// with checking.
#define GetRef(a, name, T) auto a = getReference<T>(name, #T);

template <typename Runner, typename Type> class Test_T : public Type {
 public:

  std::map<std::string, std::string> parameters;
  std::shared_ptr<Runner> runner;

  Test_T(TestConfig& config, const char* params) : Type(config) {
    runner.reset(new Runner());
    parameters = StringUtils::variadicProcess(params);
  }

  template <typename T> const T& get(std::string param) {
    auto it = parameters.find(param);
    if (it != parameters.end()) {
      return this-> template getReference<T>(param, it->second);
    }
    throw VnV::VnVExceptionBase("Parameter does not exist");
  }

};

}  // namespace VnV

#define VNVDECLAREMACRO(Type, PNAME, name) namespace VnV { namespace PNAME { namespace Type { void register_##name(); }}}
  #define VNVREGISTERMACRO(Type, PNAME, name) VnV::PNAME::Type::register_##name();



#define INJECTION_TEST_RS(PNAME, name, Runner, schema, ...)                    \
  namespace VnV {                                                              \
  namespace PNAME {                                                            \
  namespace Tests {                                                            \
  class name : public Test_T<VnV_Arg_Type(Runner), ITest> {                           \
   public:                                                                     \
    name(TestConfig& config) : Test_T<VnV_Arg_Type(Runner), ITest>(config, #__VA_ARGS__) {}         \
    TestStatus runTest(ICommunicator_ptr comm, IOutputEngine* engine,  InjectionPointType type, std::string stageId); \
  };                                                                               \
  ITest* declare_##name(TestConfig config) { return new name(config); }        \
  void register_##name() {                                                     \
      VnV::registerTest(VNV_STR(PNAME), #name, schema,declare_##name, VnV::StringUtils::variadicProcess(#__VA_ARGS__)); \
  }                                                                           \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  VnV::TestStatus VnV::PNAME::Tests::name::runTest(ICommunicator_ptr comm, VnV::IOutputEngine* engine, VnV::InjectionPointType type, std::string stageId)

#define DECLARETEST(PNAME, name) VNVDECLAREMACRO(Tests, PNAME, name)  

#define REGISTERTEST(PNAME, name) VNVREGISTERMACRO(Tests, PNAME, name) 


#define INJECTION_TEST_R(PNAME, name, runner, ...) \
  INJECTION_TEST_RS(PNAME, name, runner, R"({"type":"object"})", __VA_ARGS__)

#define INJECTION_TEST(PNAME, name, ...) \
  INJECTION_TEST_R(PNAME, name, int, __VA_ARGS__)

#endif  // ITEST_H
