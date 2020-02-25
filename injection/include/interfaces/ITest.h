#ifndef ITEST_H
#define ITEST_H

#include <map>
#include <string>
#include "json-schema.hpp"
#include "interfaces/IOutputEngine.h"

using nlohmann::json;
/**
 * @brief The TestConfig class
 */
namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };


typedef std::map<std::string, std::pair<std::string, void*>> NTV;

struct ParameterMapping {
    std::string injectionPointParameter;
    bool required;
    std::string parameterType;
};

class TestConfig {
 private:
  std::map<std::string, ParameterMapping > parameterMap;
  std::string testName;
  json additionalParameters;
  json expectedResult;

  void* mapParameter(std::string name, NTV& parameters) const;

 public:

  TestConfig(std::string name, json &usersConfig, json &testSpec);

  bool isRequired(std::string parmaeterName) const;
  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;

  std::map<std::string, void*> mapParameters(NTV& parameters) const ;


  bool isMappingValidForParameterSet(NTV &parameters) const ;

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
   * @brief print out configuration information.
   */
  void print();

  /**
   * @brief addTransform
   * @param to
   * @param from
   * @param trans
   */
  ParameterMapping getMapping(std::string testParameter) const ;

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
  TestStatus _runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId, NTV& params);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId, std::map<std::string, void*>&params) = 0;

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


typedef ITest* maker_ptr(TestConfig config);
typedef json declare_test_ptr();
void registerTest(std::string name, VnV::maker_ptr m, VnV::declare_test_ptr v);

}

#endif // ITEST_H
