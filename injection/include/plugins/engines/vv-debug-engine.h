#ifndef VV_DEBUGENGINE_HEADER
#define VV_DEBUGENGINE_HEADER

/**
 * @file vv-debug-engine.h
 */

#include <string>
#include "interfaces/ioutputengine.h"
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
   * @brief Log
   * @param log
   */
  void Log(const char * package, int stage, std::string level, std::string message);


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
   * @brief Define IO Variable
   * @param name
   */
  void Define(VariableEnum type, std::string name);

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

  std::string getIndent(int stage);

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
  void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageVal) override;


  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointStartedCallBack(std::string id, InjectionPointType type, std::string stageVal) override;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void testStartedCallBack(std::string testName ) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(bool result_) override;

  void unitTestStartedCallBack(std::string unitTestName) override;

  void unitTestFinishedCallBack(std::map<std::string,bool> &results) override;
  /**
   * @brief getOutputEngine
   * @return
   */
  IOutputEngine* getOutputEngine();
};

}  // namespace VnV

#endif
