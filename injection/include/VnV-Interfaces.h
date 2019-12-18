#ifndef VV_INTERFACE_HEADER
#define VV_INTERFACE_HEADER

/** @file VnV-Interfaces.h */

#include <map>
#include <set>
#include <string>
#include <iostream>
#include "VnV.h"
#include "json-schema.hpp"

using nlohmann::json;

/**
 *
 */
namespace VnV {

// enum class LogLevel { DEBUG, INFO, WARN, ERROR, STAGE_END, STAGE_START };

 enum class VariableEnum {Double, String, Int, Float, Long};

 enum class InjectionPointType {Single, Begin, End, Iter};
 namespace InjectionPointTypeUtils {
    std::string getType(InjectionPointType type, std::string stageId);
 }
 /**
 *@namespace convienence methods for Variable Type enum.
 */

namespace VariableEnumFactory {
    VariableEnum fromString(std::string s);
    std::string toString(VariableEnum e);
};

class LogStageRef {

};

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
  virtual void Put(std::string variableName, double& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, int& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, float& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, long& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(std::string variableName, std::string& value);

  virtual void Log(const char * packageName, int stage, std::string level, std::string message);

    /**
   * @brief Define IO variables that will be written.
   * @param type
   * @param name
   */
  virtual void Define(VariableEnum type, std::string name) = 0;

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

  virtual void print();

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageId) = 0;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void injectionPointStartedCallBack(std::string id, InjectionPointType type, std::string stageId) = 0;


  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  virtual void testStartedCallBack(std::string testName) = 0;

  /**
   * @brief stopTest
   * @param result_
   */
  virtual void testFinishedCallBack(bool result_) = 0;


  virtual void unitTestStartedCallBack(std::string unitTestName) = 0;

  virtual void unitTestFinishedCallBack(std::map<std::string,bool> &results) = 0;

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
  friend class TestStore;
    /**
   * @brief ITransform
   */
  ITransform();


  virtual ~ITransform();
private:

  /**
   * @brief Transform
   * @param ip
   * @param tp
   * @return
   */
  virtual void* Transform(std::string outputType, std::string inputType, void* ptr ) = 0;

};

class ISerializer {
public:
    ISerializer();

    ~ISerializer();

    /**
    * Return the size required for the buffer to serialize the object.
    */
    virtual std::size_t getBuffSize(void* data) = 0;

    /**
     * Pack the given void* pointer ( of class type classType ) into the buffer
     */
    virtual void* buffPack(void* data, std::string classType) = 0;

    /**
     * Unpack the buffer into a object with class type "classType". Return
     * the class in a void* pointer. We will free the new class and buffer
     * internally.
     */
    virtual void* buffUnpack(void* buffsize, std::string buffer) = 0;

};



/**
 * @brief The TestConfig class
 */

struct ParameterMapping {
    std::string injectionPointParameter;
    bool required;
    std::string parameterType;
};

class TestConfig {
 private:
  std::map<std::string, ParameterMapping > parameterMap;
  std::map<std::string, bool> requiredMap;
  std::map<std::string, std::string> parameterTypeMap;
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
  virtual std::map<std::string, bool> run(IOutputEngine *engine) = 0;


};
typedef IUnitTester* tester_ptr();
typedef ITest* maker_ptr(TestConfig config);
typedef ISerializer* serializer_ptr();
typedef json declare_test_ptr();
typedef json declare_transform_ptr();
typedef json declare_serializer_ptr();
typedef ITransform* trans_ptr();
typedef OutputEngineManager* engine_register_ptr();
typedef json options_schema_ptr();
typedef void options_callback_ptr(json &info);
}  // namespace VnV

#ifdef __cplusplus
#  define EXTERNC extern "C"
#else
#  define EXTERNC
#endif
EXTERNC void VnV_registerTest(std::string name, VnV::maker_ptr m, VnV::declare_test_ptr v);
EXTERNC void VnV_registerTransform(std::string name, VnV::trans_ptr t, VnV::declare_transform_ptr v);
EXTERNC void VnV_registerSerializer(std::string name, VnV::serializer_ptr t, VnV::declare_serializer_ptr v);
EXTERNC void VnV_registerEngine(std::string name, VnV::engine_register_ptr r);
EXTERNC void VnV_registerUnitTester(std::string name, VnV::tester_ptr ptr);
EXTERNC void VnV_registerOptions(std::string name, VnV::options_schema_ptr s, VnV::options_callback_ptr v);
#undef EXTERNC

#endif
