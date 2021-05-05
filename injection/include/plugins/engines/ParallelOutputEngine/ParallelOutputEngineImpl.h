#ifndef VV_PARALLELENGINE_HEADER
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
           const double& value,const MetaData& m) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const long long& value,const MetaData& m) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put( std::string variableName, const bool& value,const MetaData& m) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, const json& value,const MetaData& m) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put( std::string variableName,
           const std::string& value,const MetaData& m) override;

  void Put(std::string variableName,
           IDataType_ptr data,
           const MetaData& m ) override;

  void PutGlobalArray(
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                                 const MetaData& m) override
  {}

  std::string getIndent(int stage);

  /**
   * @brief Get the configuration Schema for the Debug engine.
   */
  json getConfigurationSchema() override;

  /**
   * @brief finalize
   */
  void finalize(ICommunicator_ptr worldComm) override;

  /**
   * @brief set
   * @param config
   */
  void setFromJson(ICommunicator_ptr worldComm,json& config) override;

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointEndedCallBack(std::string id,
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
  void testStartedCallBack(std::string packageName,
                           std::string testName, bool internal) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack( IUnitTest* tester) override;


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
