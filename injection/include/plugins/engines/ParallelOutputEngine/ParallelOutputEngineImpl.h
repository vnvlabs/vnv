#ifndef VV_PARALLELENGINE_HEADER
#define VV_PARALLELENGINE_HEADER

/**
 * @file vv-debug-engine.h
 */

#include <memory>
#include <string>

#include "Router.h"
#include "interfaces/IOutputEngine.h"

/**
 * VnV Namespace
 */
namespace VnV {
namespace PACKAGENAME {
namespace Engines {

enum class RouterAction { PUSH, POP, IGNORE };

class ParallelEngine : public OutputEngineManager {
 private:
  // Router* router;
  std::map<int, std::shared_ptr<Router>> routerMap;

 public:
  /**
   * @brief ParallelgEngine
   */
  ParallelEngine();

  /**
   * @brief Log
   * @param log
   */
  void Log(VnV_Comm comm, const char* package, int stage, std::string level,
           std::string message);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName,
           const double& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName, const int& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName,
           const float& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName, const long& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName, const bool& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName, const json& value) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(VnV_Comm comm, std::string variableName, const std::string& value);

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
  void injectionPointEndedCallBack(VnV_Comm comm, std::string id,
                                   InjectionPointType type,
                                   std::string stageVal) override;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointStartedCallBack(VnV_Comm comm, std::string id,
                                     InjectionPointType type,
                                     std::string stageVal) override;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void testStartedCallBack(VnV_Comm comm, std::string testName) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(VnV_Comm comm, bool result_) override;

  void unitTestStartedCallBack(VnV_Comm comm,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(VnV_Comm comm, IUnitTest* tester) override;

  /**
   * @brief getOutputEngine
   * @return
   */
  Nodes::IRootNode* readFromFile(std::string file) override {
    throw VnV::VnVExceptionBase(
        "Read From File Not implemented for Parallel Output Engine");
  }

  std::string print() override {
    printf("Print not implemented for Parallel Output Engine");
  }

  std::shared_ptr<Router> getRouter(VnV_Comm comm,
                                    RouterAction action = RouterAction::IGNORE);
};

}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
#endif
