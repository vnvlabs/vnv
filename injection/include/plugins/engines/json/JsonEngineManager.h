#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include "interfaces/IOutputEngine.h"
namespace VnV {
namespace PACKAGENAME {
namespace Engines {

class JsonEngineManager : public OutputEngineManager {
  json mainJson;
  json::json_pointer ptr;
  long id = 0;
  std::string outputFile;

  std::string getId();

  void append(json& jsonOb);

  void pop(int num);

  void push(json& jsonOb);

  void append(json::json_pointer ptr);

  void push(json& jsonOb, json::json_pointer ptr);

  std::string Dump(int d);

 public:
  JsonEngineManager();

#define LTypes X(double) X(int) X(bool) X(float) X(long) X(std::string) X(json)
#define X(type) \
  void Put(VnV_Comm comm, std::string variableName, const type& value);
  LTypes
#undef X
#undef LTypes

      void
      Log(VnV_Comm comm, const char* package, int stage, std::string level,
          std::string message);

  json getConfigurationSchema();

  void finalize();

  void setFromJson(json& config);

  void injectionPointEndedCallBack(VnV_Comm comm, std::string id,
                                   InjectionPointType type,
                                   std::string stageVal);

  void injectionPointStartedCallBack(VnV_Comm comm, std::string packageName, std::string id,
                                     InjectionPointType type,
                                     std::string stageVal) override;

  void testStartedCallBack(VnV_Comm comm, std::string packageName, std::string testName, bool internal) override;

  void testFinishedCallBack(VnV_Comm comm, bool result_);

  void unitTestStartedCallBack(VnV_Comm comm, std::string packageName, std::string unitTestName);

  void unitTestFinishedCallBack(VnV_Comm comm, IUnitTest* tester);
  void dataTypeStartedCallBack(VnV_Comm /** comm **/,
                               std::string variableName, std::string dtype) override;
  void dataTypeEndedCallBack(VnV_Comm /** comm **/, std::string variableName) override;

  Nodes::IRootNode* readFromFile(std::string file, long& idCounter);

  // IInternalOutputEngine interface
  std::string print();
};
}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
#endif  // JSONENGINEMANAGER_H
