#ifndef VV_DEBUGENGINE_HEADER
#define VV_DEBUGENGINE_HEADER

/**
 * @file vv-debug-engine.h
 */

#include <string>

#include "vv-output.h"
/**
 * VnV Namespace
 */
namespace VnV {

/**
 * @brief The DebugEngine class
 */
class DebugEngine : public IOutputEngine {
 public:
  /**
   * @brief DebugEngine
   */
  DebugEngine();

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, double& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, int& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, float& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, long& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, std::string& value);

  /**
   * @brief DefineDouble
   * @param name
   */
  void DefineDouble(std::string name);

  /**
   * @brief DefineFloat
   * @param name
   */
  void DefineFloat(std::string name);

  /**
   * @brief DefineInt
   * @param name
   */
  void DefineInt(std::string name);

  /**
   * @brief DefineLong
   * @param name
   */
  void DefineLong(std::string name);

  /**
   * @brief DefineString
   * @param name
   */
  void DefineString(std::string name);
};

/**
 * @brief The DebugEngineWrapper class
 */
class DebugEngineWrapper : public OutputEngineManager {
 private:
  DebugEngine* debugEngine; /**< @todo */

 public:
  /**
   * @brief DebugEngineWrapper
   */
  DebugEngineWrapper();

  /**
   * @brief Get the configuration Schema for the Debug engine. 
   */
  json getConfigurationSchema() override;


  /**
   * @brief finalize
   */
  void finalize();

  /**
   * @brief set
   * @param config
   */
  void set(json& config);

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  void endInjectionPoint(std::string id, int stageVal);

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void startInjectionPoint(std::string id, int stageVal);

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void startTest(std::string testName, int testStageVal);

  /**
   * @brief stopTest
   * @param result_
   */
  void stopTest(bool result_);

  /**
   * @brief getOutputEngine
   * @return
   */
  IOutputEngine* getOutputEngine();
};

}  // namespace VnV

#endif
