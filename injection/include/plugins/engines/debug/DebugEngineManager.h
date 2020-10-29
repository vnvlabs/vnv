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
           const double& value,const MetaData& m) override;

  void Put( std::string variableName, const long long& value,const MetaData& m) override;

  void Put( std::string variableName, const json& value,const MetaData& m) override;

  void Put( std::string variableName,
           const std::string& value,const MetaData& m) override;

  void Put( std::string variableName, const bool& value, const MetaData& m) override;

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                                 const MetaData& m,
                      int onlyOne=-1 ) override
  {}

  void Log(ICommunicator_ptr comm, const char* package, int stage, std::string level,
           std::string message) override;

  json getConfigurationSchema() override;

  void finalize(ICommunicator_ptr worldComm) override;

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
  void dataTypeStartedCallBack(std::string variableName,
                               long long dtype,const MetaData& m) override;
  void dataTypeEndedCallBack(
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
