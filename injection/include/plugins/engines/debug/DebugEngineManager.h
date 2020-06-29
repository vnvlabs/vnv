#ifndef DEBUGENGINEMANAGER_H
#define DEBUGENGINEMANAGER_H

#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace PACKAGENAME {
namespace Engines {

class DebugEngineManager : public OutputEngineManager {
 public:
  DebugEngineManager();
  ~DebugEngineManager();

  void Put(VnV_Comm comm, std::string variableName,
           const double& value) override;

  void Put(VnV_Comm comm, std::string variableName, const int& value) override;

  void Put(VnV_Comm comm, std::string variableName,
           const float& value) override;

  void Put(VnV_Comm comm, std::string variableName, const long& value) override;

  void Put(VnV_Comm comm, std::string variableName, const json& value) override;

  void Put(VnV_Comm comm, std::string variableName,
           const std::string& value) override;

  void Put(VnV_Comm comm, std::string variableName, const bool& value) override;

  void Log(VnV_Comm comm, const char* package, int stage, std::string level,
           std::string message) override;

  json getConfigurationSchema() override;

  void finalize() override;

  void setFromJson(json& config) override;

  void injectionPointEndedCallBack(VnV_Comm comm, std::string id,
                                   InjectionPointType type,
                                   std::string stageVal) override;

  void injectionPointStartedCallBack(VnV_Comm comm, std::string id,
                                     InjectionPointType type,
                                     std::string stageVal) override;

  void testStartedCallBack(VnV_Comm comm, std::string testName) override;

  void testFinishedCallBack(VnV_Comm comm, bool result_) override;
  void dataTypeStartedCallBack(VnV_Comm /** comm **/,
                               std::string variableName, std::string dtype) override;
  void dataTypeEndedCallBack(VnV_Comm /** comm **/, std::string variableName) override;

  void unitTestStartedCallBack(VnV_Comm comm,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(VnV_Comm comm, IUnitTest* tester) override;

  Nodes::IRootNode* readFromFile(std::string file) override;

  std::string print() override;
};

}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
#endif  // DEBUGENGINEMANAGER_H
