#ifndef JsonStreamingEngineManager_H
#define JsonStreamingEngineManager_H

#include <chrono>
#include <iostream>
#include <set>

#include "base/CommMapper.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class JsonStream {
public:
  virtual void newComm(long id, json obj) = 0;
  virtual void write(long id, json obj) = 0 ;

};

class JsonStreamingEngineManager : public OutputEngineManager {
 protected:
  bool inMemory;

  std::set<long> commids;

  std::map<long, json::json_pointer> ptr;
  long id = 0;
  std::string outputFile;
  
  std::shared_ptr<JsonStream> stream;

  CommMapper commMapper;

  std::string getId();
  
  virtual void write(json& j) {
    stream->write(comm->uniqueId(), j);
  }

  virtual void setComm(ICommunicator_ptr comm) {
    setCommunicator(comm);
    commMapper.logComm(comm);
    long id = comm->uniqueId();
    
    if (comm->Rank() == getRoot() && commids.find(id) == commids.end() ) {
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
      stream->newComm(id, nJson);
      commids.insert(id);
    }
  }

 public:
  JsonStreamingEngineManager(std::shared_ptr<JsonStream> stream_) : stream(stream_) {}

#define LTypes X(double) X(bool) X(long long) X(std::string) X(json)
#define X(type)                                                            \
  void Put(std::string variableName, const type& value, const MetaData& m) \
      override;
  LTypes
#undef X
#undef LTypes

  void Put(std::string variableName, IDataType_ptr data, const MetaData& m);


  void PutGlobalArray(long long dtype, std::string variableName,
                      IDataType_vec data, std::vector<int> gsizes,
                      std::vector<int> sizes, std::vector<int> offset,
                      const MetaData& m) override;

  void Log(ICommunicator_ptr comm, const char* package, int stage, std::string level,
           std::string message) override;

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
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // JsonStreamingEngineManager_H
