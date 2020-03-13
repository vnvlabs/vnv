#ifndef JSONOUTPUTENGINE_H
#define JSONOUTPUTENGINE_H

/**
 * @file vv-debug-engine.h
 */

#include <string>
#include "interfaces/IOutputEngine.h"
#include <stack>

/**
 * VnV Namespace
 */
namespace VnV {

/**
 * @brief The JsonEngine class
 */
class JsonEngine : public IOutputEngine {
    json mainJson ;
    json::json_pointer ptr;


public:
  /**
   * @brief JsonEngine
   */
  JsonEngine();

  void append(json &json);
  void push(json &json, json::json_pointer ptr);
  void push(json &jsonr);
  void pop(int num);

  /**
   * @brief Log
   * @param log
   */
  void Log(const char * package, int stage, std::string level, std::string message) override;


  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName,const  double& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const int& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const float& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const long& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const std::string& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const json& value) override;

  /**
   * @brief Define IO Variable
   * @param name
   */
  void Define(VariableEnum type, std::string name);


  std::string Dump(int level);

};

/**
 * @brief The JsonEngineWrapper class
 */
class JsonEngineWrapper : public OutputEngineManager {
 private:
  JsonEngine* jsonEngine; /**< @todo */
  std::string outputFile = "";
 public:
  /**
   * @brief JsonEngineWrapper
   */
  JsonEngineWrapper();

  std::string getIndent(int stage);

  /**
   * @brief Get the configuration Schema for the Json engine.
   */
  json getConfigurationSchema() override;


  /**
   * @brief finalize
   */
  void finalize() override;

  /**
   * @brief set
   * @param config
   */
  void set(json& config) override;

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

  void documentationStartedCallBack(std::string pname, std::string id) override;

  void documentationEndedCallBack(std::string pname, std::string id) override;


  /**
   * @brief getOutputEngine
   * @return
   */
  IOutputEngine* getOutputEngine();
};

}  // namespace VnV
#endif // JSONOUTPUTENGINE_H
