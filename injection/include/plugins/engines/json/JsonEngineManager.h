#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include <iostream>

#include "interfaces/IOutputEngine.h"
#include "interfaces/ICommunicator.h"
#include "base/CommMapper.h"
#include <set>
#include <chrono>


namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class JsonEngineManager : public OutputEngineManager {
  json mainJson;
  std::map<long, json::json_pointer> ptr;
  long id = 0;
  ICommunicator_ptr currComm = nullptr;
  std::string outputFile;

  CommMapper commMapper;

  std::string getId();

  void append(json& jsonOb);

  void pop(int num);

  void add(std::string key, const json& ob);

  void push(json& jsonOb);

  void append(json::json_pointer ptr);

  void push(json& jsonOb, json::json_pointer ptr);

  std::string Dump(int d);

  void setComm(ICommunicator_ptr comm) {
    currComm = comm;
    commMapper.logComm(comm);
    long id = currComm->uniqueId();
    std::string cid = std::to_string(id);
    if (currComm->Rank() == getRoot(comm) && !mainJson.contains(cid)) {
       json nJson;
       nJson["info"] = json::object();
       nJson["info"]["title"] = "VnV Simulation Report";
       nJson["info"]["date"] =
          std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
       nJson["info"]["id"] = getId();
       nJson["info"]["name"] = "info";
       nJson["children"] = json::array();
       nJson["comm"] = json::object();

       mainJson[cid] = nJson;
       ptr[currComm->uniqueId()] = json::json_pointer("/children");
    }
  }

 public:
  JsonEngineManager();

#define LTypes X(double) X(bool) X(long long) X(std::string) X(json)
#define X(type) \
  void Put(std::string variableName, const type& value,const MetaData& m) override;
  LTypes
#undef X
#undef LTypes


  void WriteDataArray(std::string variableName, IDataType_vec &data, std::vector<int> &shape, const MetaData &m);

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                                 const MetaData& m,
                              int onlyOne=-1 ) override;

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

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(ICommunicator_ptr comm, IUnitTest* tester) override;
  void dataTypeStartedCallBack( std::string variableName,
                               long long dtype,const MetaData& m) override;
  void dataTypeEndedCallBack(
                             std::string variableName) override;

  Nodes::IRootNode* readFromFile(std::string file, long& idCounter) override;

  // IInternalOutputEngine interface
  std::string print() override;
};
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // JSONENGINEMANAGER_H
