#ifndef VV_DEBUGENGINE_HEADER
#define VV_DEBUGENGINE_HEADER

/**
 * @file vv-debug-engine.h
 */

#include <string>
#include "interfaces/IOutputEngine.h"
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
  void Log(VnV_Comm comm,const char * package, int stage, std::string level, std::string message);


  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const double& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const int& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const float& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const long& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const std::string& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm,std::string variableName, const json& value) override;

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
  void injectionPointEndedCallBack(VnV_Comm comm, std::string id, InjectionPointType type, std::string stageVal) override;


  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointStartedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageVal) override;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void testStartedCallBack(VnV_Comm comm,std::string testName ) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(VnV_Comm comm,bool result_) override;

  void unitTestStartedCallBack(VnV_Comm comm,std::string unitTestName) override;

  void unitTestFinishedCallBack(VnV_Comm comm,std::map<std::string,bool> &results) override;

  void documentationStartedCallBack(VnV_Comm comm,std::string pname, std::string id) override;

  void documentationEndedCallBack(VnV_Comm comm,std::string pname, std::string id) override;


  /**
   * @brief getOutputEngine
   * @return
   */
  IOutputEngine* getOutputEngine();
};

}  // namespace VnV

#endif
