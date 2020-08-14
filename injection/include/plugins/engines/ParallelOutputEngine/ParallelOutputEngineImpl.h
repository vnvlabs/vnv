﻿#ifndef VV_PARALLELENGINE_HEADER
#define VV_PARALLELENGINE_HEADER

/**
 * @file vv-debug-engine.h
 */

#include <memory>
#include <string>

#include "Router.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
/**
 * VnV Namespace
 */
namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

enum class RouterAction { PUSH, POP, IGNORE };

class ParallelEngine : public OutputEngineManager {
 private:
  // Router* router;
  std::map<int, std::shared_ptr<Router>> routerMap;
  ICommunicator_ptr currComm;


 public:
  /**
   * @brief ParallelgEngine
   */
  ParallelEngine();

  /**
   * @brief Log
   * @param log
   */
  void Log(ICommunicator_ptr comm, const char* package, int stage, std::string level,
           std::string message) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName,
           const double& value) override;

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
  void Put( std::string variableName, const long& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put( std::string variableName, const bool& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const json& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put( std::string variableName,
           const std::string& value) override;

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                      int onlyOne=-1 ) override
  {}

  std::string getIndent(int stage);

  /**
   * @brief Get the configuration Schema for the Debug engine.
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
  void setFromJson(json& config) override;

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointEndedCallBack(ICommunicator_ptr comm, std::string id,
                                   InjectionPointType type,
                                   std::string stageVal) override;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                                     std::string id, InjectionPointType type,
                                     std::string stageVal) override;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void testStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                           std::string testName, bool internal) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(ICommunicator_ptr comm, bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(ICommunicator_ptr comm, IUnitTest* tester) override;

  void dataTypeStartedCallBack(ICommunicator_ptr /** comm **/, std::string variableName,
                               long long dtype) override;
  void dataTypeEndedCallBack(ICommunicator_ptr /** comm **/,
                             std::string variableName) override;

  /**
   * @brief getOutputEngine
   * @return
   */
  Nodes::IRootNode* readFromFile(std::string file, long& idCOunter) override {
    throw VnV::VnVExceptionBase(
        "Read From File Not implemented for Parallel Output Engine");
  }

  std::string print() override {
    throw VnV::VnVExceptionBase(
        "Print not implemented for Parallel Output Engine");
  }

  std::shared_ptr<Router> getRouter(
                                    RouterAction action = RouterAction::IGNORE);
};

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif
