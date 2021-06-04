#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include <chrono>
#include <iostream>
#include <set>

#include "base/CommMapper.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace Json {

VnV::Nodes::IRootNode* parse(std::string filename, long& idCounter);

class JsonEngineManager : public OutputEngineManager {
 protected:
  bool inMemory;

  json mainJson;
  std::map<long, json::json_pointer> ptr;
  long id = 0;
  std::string outputFile;

  CommMapper commMapper;

  std::string getId();

  virtual void append(json& jsonOb);

  virtual void pop(int num);

  virtual void add(std::string key, const json& ob);

  virtual void push(json& jsonOb);

  virtual void append(json::json_pointer ptr);

  virtual void push(json& jsonOb, json::json_pointer ptr);

  virtual void setComm(ICommunicator_ptr comm) {
    setCommunicator(comm);
    commMapper.logComm(comm);
    long id = comm->uniqueId();
    std::string cid = std::to_string(id);
    if (comm->Rank() == getRoot() && !mainJson.contains(cid)) {
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
      ptr[comm->uniqueId()] = json::json_pointer("/children");
    }
  }

  std::string Dump(int d);

 public:
  JsonEngineManager();

#define LTypes X(double) X(bool) X(long long) X(std::string) X(json)
#define X(type)                                                            \
  void Put(std::string variableName, const type& value, const MetaData& m) \
      override;
  LTypes
#undef X
#undef LTypes

      void
      Put(std::string variableName, IDataType_ptr data, const MetaData& m);

  void PutGlobalArray(long long dtype, std::string variableName,
                      IDataType_vec data, std::vector<int> gsizes,
                      std::vector<int> sizes, std::vector<int> offset,
                      const MetaData& m) override;

  void Log(ICommunicator_ptr comm, const char* package, int stage,
           std::string level, std::string message) override;

  json getConfigurationSchema() override;

  void finalize(ICommunicator_ptr worldComm) override;

  void setFromJson(ICommunicator_ptr worldComm, json& config) override;

  void injectionPointEndedCallBack(std::string id, InjectionPointType type,
                                   std::string stageVal) override;

  void injectionPointStartedCallBack(ICommunicator_ptr comm,
                                     std::string packageName, std::string id,
                                     InjectionPointType type,
                                     std::string stageVal) override;

  void testStartedCallBack(std::string packageName, std::string testName,
                           bool internal) override;

  void testFinishedCallBack(bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(IUnitTest* tester) override;

  Nodes::IRootNode* readFromFile(std::string file, long& idCounter) override;

  // IInternalOutputEngine interface
  std::string print() override;
};
}
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // JSONENGINEMANAGER_H
