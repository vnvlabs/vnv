#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include "interfaces/IOutputEngine.h"
namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class JsonEngineManager : public OutputEngineManager {
  json mainJson;
  json::json_pointer ptr;
  long id = 0;
  ICommunicator_ptr currComm = nullptr;
  std::string outputFile;

  std::string getId();

  void append(json& jsonOb);

  void pop(int num);

  void push(json& jsonOb);

  void append(json::json_pointer ptr);

  void push(json& jsonOb, json::json_pointer ptr);

  std::string Dump(int d);

  void setComm(ICommunicator_ptr comm) {
    currComm = comm;
  }

 public:
  JsonEngineManager();

#define LTypes X(double) X(int) X(bool) X(long) X(std::string) X(json)
#define X(type) \
  void Put(std::string variableName, const type& value) override;
  LTypes
#undef X
#undef LTypes


  void WriteDataArray(std::string variableName, IDataType_vec &data, std::vector<int> &shape);

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                              int onlyOne=-1 ) override;

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

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(ICommunicator_ptr comm, IUnitTest* tester) override;
  void dataTypeStartedCallBack(ICommunicator_ptr /** comm **/, std::string variableName,
                               long long dtype) override;
  void dataTypeEndedCallBack(ICommunicator_ptr /** comm **/,
                             std::string variableName) override;

  Nodes::IRootNode* readFromFile(std::string file, long& idCounter) override;

  // IInternalOutputEngine interface
  std::string print() override;
};
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // JSONENGINEMANAGER_H
