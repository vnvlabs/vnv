#ifndef DEBUGENGINEMANAGER_H
#define DEBUGENGINEMANAGER_H

#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class DebugEngineManager : public OutputEngineManager {
 public:
  DebugEngineManager();
  ~DebugEngineManager();

  void Put( std::string variableName,
           const double& value) override;

  void Put(std::string variableName, const int& value) override;


  void Put( std::string variableName, const long& value) override;

  void Put( std::string variableName, const json& value) override;

  void Put( std::string variableName,
           const std::string& value) override;

  void Put( std::string variableName, const bool& value) override;

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                      int onlyOne=-1 ) override
  {}

  void Log(ICommunicator_ptr comm, const char* package, int stage, std::string level,
           std::string message) override;

  json getConfigurationSchema() override;

  void finalize() override;

  void setFromJson(json& config) override;

  void injectionPointEndedCallBack(ICommunicator_ptr comm, std::string id,
                                   InjectionPointType type,
                                   std::string stageVal) override;

  void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                                     std::string id, InjectionPointType type,
                                     std::string stageVal) override;

  void testStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                           std::string testName, bool internal) override;

  void testFinishedCallBack(ICommunicator_ptr comm, bool result_) override;
  void dataTypeStartedCallBack(ICommunicator_ptr /** comm **/, std::string variableName,
                               long long dtype) override;
  void dataTypeEndedCallBack(ICommunicator_ptr /** comm **/,
                             std::string variableName) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(ICommunicator_ptr comm, IUnitTest* tester) override;

  Nodes::IRootNode* readFromFile(std::string file, long&) override;

  std::string print() override;
};

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // DEBUGENGINEMANAGER_H
