#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>
#include "json-schema.hpp"
/**
 * @brief The IOutputEngine class
 */
using nlohmann::json;

namespace VnV {

enum class InjectionPointType {Single, Begin, End, Iter};
namespace InjectionPointTypeUtils {
   std::string getType(InjectionPointType type, std::string stageId);
}

enum class VariableEnum {Double, String, Int, Float, Long};
namespace VariableEnumFactory {
    VariableEnum fromString(std::string s);
    std::string toString(VariableEnum e);
};

class IOutputEngine {

public:

   std::string getIndent(int stage);

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

typedef OutputEngineManager* engine_register_ptr();
void registerEngine(std::string name, VnV::engine_register_ptr r);

}



#endif // IOUTPUTENGINE_H
