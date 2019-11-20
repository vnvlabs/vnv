#ifndef VV_INTERFACE_HEADER
#define VV_INTERFACE_HEADER

/** @file VnV-Interfaces.h */

#include <map>
#include <set>
#include <string>
#include <iostream>

#include "json-schema.hpp"

using nlohmann::json;

/**
 *
 */
namespace VnV {

/**
 * @brief The IOutputEngine class
 */
class IOutputEngine {
 public:
  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, double& value) = 0;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, int& value) = 0;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, float& value) = 0;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, long& value) = 0;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, std::string& value) = 0;

  /**
   * @brief DefineDouble
   * @param name
   */
  virtual void DefineDouble(std::string name) = 0;

  /**
   * @brief DefineFloat
   * @param name
   */
  virtual void DefineFloat(std::string name) = 0;

  /**
   * @brief DefineInt
   * @param name
   */
  virtual void DefineInt(std::string name) = 0;

  /**
   * @brief DefineLong
   * @param name
   */
  virtual void DefineLong(std::string name) = 0;

  /**
   * @brief DefineString
   * @param name
   */
  virtual void DefineString(std::string name) = 0;

  void Define(std::string type, std::string name);

  /**
   * @brief ~IOutputEngine
   */
  virtual ~IOutputEngine();
};
/**
 * @brief The OutputEngineManager class
 */
class OutputEngineManager {
 public:
  /**
   * @brief _set
   * @param configuration
   */
  void _set(json& configuration);
  /**
   * @brief set
   * @param configuration
   */
  virtual void set(json& configuration) = 0;

  /**
   * @brief getConfigurationSchema
   * @return
   */
  virtual json getConfigurationSchema();

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void endInjectionPoint(std::string id, int stageVal) = 0;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void startInjectionPoint(std::string id, int stageVal) = 0;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  virtual void startTest(std::string testName, int testStageVal) = 0;

  /**
   * @brief stopTest
   * @param result_
   */
  virtual void stopTest(bool result_) = 0;

  /**
   * @brief finalize
   */
  virtual void finalize() = 0;

  /**
   * @brief getOutputEngine
   * @return
   */
  virtual IOutputEngine* getOutputEngine() = 0;

  /**
   * @brief ~OutputEngineManager
   */
  virtual ~OutputEngineManager();
};

// Some simplifying typedefs.
/**
 * @brief NTV
 */
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

/**
 * @brief NT
 */
typedef std::map<std::string, std::string> NT;

/**
 * @brief The TestStatus enum
 */
enum TestStatus { SUCCESS, FAILURE, NOTRUN };

// Transform interface.
/**
 * @brief The ITransform class
 */
class ITransform {
 public:
  /**
   * @brief ITransform
   */
  ITransform();
  void addInnerTransform(std::string trans);

  void* Transform(std::pair<std::string,void*> ip, std::string tp);

private:
  std::vector<std::string> innerTransforms;

  /**
   * @brief Transform
   * @param ip
   * @param tp
   * @return
   */
  virtual void* _Transform(std::pair<std::string, void*> ip, std::string tp) = 0;

};
/**
 * @brief The TestStageConfig class
 */
class TestStageConfig {
 private:
  int injectionPointStageId;
  int testStageId;

  std::map<std::string, std::pair<std::string, std::vector<std::string>>>
      transforms;  // maps testParameter -> ip_parameter & Transform

 public:
  /**
   * @brief addTransform
   * @param to
   * @param from
   * @param trans
   */
  void addTransform(std::string to, std::string from, std::vector<std::string> trans);

  /**
   * @brief getInjectionPointStageId
   * @return
   */
  int getInjectionPointStageId();

  /**
   * @brief getTestStageId
   * @return
   */
  int getTestStageId();

  /**
   * @brief setInjectionPointStageId
   * @param id
   */
  void setInjectionPointStageId(int id);

  /**
   * @brief setTestStageId
   * @param id
   */
  void setTestStageId(int id);

  /**
   * @brief getTransform
   * @return
   */
  std::pair<std::string, std::shared_ptr<ITransform>> getTransform(std::string);

};

/**
 * @brief The TestConfig class
 */
class TestConfig {
 private:
  std::string testName;
  std::map<int, TestStageConfig> stages;
  std::set<std::string> scopes;
  json additionalParameters;
  json expectedResult;

 public:

  TestConfig(std::string name, json &config);

  /**
   * @brief runOnStage
   * @param stage
   * @return
   */
  bool runOnStage(int stage) const ;

  /**
   * @brief addTestStage
   * @param config
   */
  void addTestStage(TestStageConfig config);

  /**
   * @brief getStage
   * @param stage
   * @return
   */
  TestStageConfig getStage(int stage) const ;

  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;


  const json& getExpectedResult() const;
  /**
   * @brief setName
   * @param name
   */
  void setName(std::string name);

  /**
   * @brief getName
   * @return
   */
  std::string getName() const ;

  /**
   * @brief getStages
   * @return
   */
  std::map<int, TestStageConfig>& getStages();
};

/**
 * @brief The ITest class
 */
class ITest {
 public:
  /**
   * @brief ITest
   */
  ITest(TestConfig &config);

  /**
   * @brief getValidationSchema
   */
  virtual json getValidationSchema();

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
  TestStatus _runTest(IOutputEngine* engine, int stageVal, NTV& params);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(IOutputEngine* engine, int stage, NTV& params) = 0;

  /**
   * @brief getConfigurationJson
   * @return
   */
  const json& getConfigurationJson() const ;

  /**
   * @brief getExpectedResultJson
   * @return
   */
  const json& getExpectedResultJson() const ;

  /**
   * @brief getTestStage
   * @param stage
   * @return
   */
  int getTestStage(int stage) const ;

 private:
  const TestConfig &m_config;
  NT m_parameters;

  /**
   * @brief carefull_cast
   * @tparam T class type
   * @param stage Test Stage
   * @param parameterName The name of the parameter to convert
   * @param parameters The Map of parameters passed to the Test by the injection
   * point.
   */

protected:

  template <typename T>
  T* carefull_cast(int stage, std::string parameterName, NTV& parameters);

};

/**
 * @brief The IUnitTester class
 *
 * Unit Tester is a function that contains InjectionPoint calls primarily for
 *	the purpose of unit testing. The idea here is that a test library,
 *engine, etc. can set up a bunch of injection points to be used for testing at
 *	runtime. For example,
 */
class IUnitTester {
 public:
  /**
   * @brief IUnitTester
   */
  IUnitTester();

  /**
   * @brief ~IUnitTester
   */
  virtual ~IUnitTester();

  /**
   * @brief run
   */
  virtual void run() = 0;

  /**
   * @brief getInputJson
   * @return
   */
  virtual std::string getInputJson() = 0;

  /**
   * @brief verifyResult
   * @param resultsEngine
   * @return
   */
  virtual bool verifyResult(IOutputEngine* resultsEngine) = 0;
};

template <typename T>
T* ITest::carefull_cast(int stage, std::string parameterName, NTV& parameters) {
  // First, make sure "parameterName" is a test parameter and, if it is, get its
  // type

  auto test_parameter = m_parameters.find(parameterName);
  if (test_parameter == m_parameters.end()) {
    throw " This is not a test parameter ";
  }


  for (auto it: parameters) {
      std::cout << it.first << " " << it.second.first << std::endl;;
  }
  // Next, get the transform. If one exists, it is returned, else return
  // "parameterName,DefaultTransform">
  std::pair<std::string, std::shared_ptr<ITransform>> trans =
      m_config.getStage(stage).getTransform(parameterName);

  auto ip_parameter = parameters.find(trans.first);

  if (ip_parameter == parameters.end()) {
    throw "A injection point parameter with the transform name does not exist";
  }

  // Transform the injection point parameter into the test parameter
  void* tptr =
      trans.second->Transform(ip_parameter->second, test_parameter->second);

  // Finally, cast it to the correct type;
  return (T*)tptr;
}

typedef IUnitTester* tester_ptr();
typedef ITest* maker_ptr(TestConfig config);
typedef json declare_test_ptr();
typedef ITransform* trans_ptr();
typedef OutputEngineManager* engine_register_ptr();

}  // namespace VnV

#ifdef __cplusplus
#  define EXTERNC extern "C"
#else
#  define EXTERNC
#endif
EXTERNC void VnV_registerTest(std::string name, VnV::maker_ptr m,
                              VnV::declare_test_ptr v);
EXTERNC void VnV_registerTransform(std::string name, VnV::trans_ptr t);
EXTERNC void VnV_registerEngine(std::string name, VnV::engine_register_ptr r);
EXTERNC void VnV_registerUnitTester(std::string name, VnV::tester_ptr ptr);

#undef EXTERNC

#endif
