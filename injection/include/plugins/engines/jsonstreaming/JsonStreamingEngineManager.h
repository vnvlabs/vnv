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

std::shared_ptr<Nodes::IRootNode> parseJsonFileStream(long& idCounter, std::string filestub);

namespace JSD {
constexpr auto outputFile = "outputFile";
constexpr auto extension = ".jstream";



static json __json_engine_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv-debug-schema.json",
    "title": "Json Engine Input Schema",
    "description": "Schema for the debug engine",
    "type": "object",
    "parameters" : {
       "outputFile" : {"type":"string"},
       "inMemory" : {"type" : "boolean" , "default" : true }
     }
}
)"_json;

#define NTYPES                                                             \
  X(id)                                                                    \
  X(name) X(package) X(value) X(shape) X(node) X(meta) X(comm) X(children) \
      X(key) X(stage) X(message) X(internal) X(description) X(result)      \
          X(stageId) X(level) X(dtype) X(endid) X(results) X(time) X(spec) \
          X(commList) X(testuid)
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES

}  // namespace JSD

namespace JSN {
#define NTYPES                                                          \
  X(log)                                                                \
  X(shape) X(dataTypeStarted) X(dataTypeEnded) X(injectionPointStarted) \
      X(injectionPointEnded) X(injectionPointIterStarted)               \
          X(injectionPointIterEnded) X(testStarted) X(testFinished)     \
          X(unitTestStarted) X(unitTestFinished) X(commInfo) X(info)    
          
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES
}  // namespace JSN


class JsonEntry {
  public:
  
  long id;
  json j;
  JsonEntry(const json& j_, long id_) : j(j_), id(id_) {}
};


class JsonIterator {
public:

  JsonIterator();
  virtual bool hasNext() const = 0;
  virtual bool isDone() const = 0;
  virtual JsonEntry next()  = 0; 
  virtual long peekId() const = 0; 
  virtual ~JsonIterator();

  virtual void push(std::shared_ptr<Nodes::DataBase> d) = 0;

  virtual std::shared_ptr<Nodes::DataBase> pop() = 0; 

  virtual std::shared_ptr<Nodes::DataBase> top()=0;

};

class JsonStream {
public:
  virtual void initialize(std::string filestub) = 0;
  virtual void finalize() = 0;
  virtual void newComm(long id, json obj) = 0;
  virtual void write(long id, json obj, long jid) = 0 ;
  JsonStream() {};
  virtual ~JsonStream() {};
};

class JsonStreamReader {
  public:
  virtual JsonEntry next() = 0;
  virtual bool hasNext() = 0;
  virtual bool isDone() = 0;
  virtual void push(std::shared_ptr<Nodes::DataBase> d) = 0;
  virtual std::shared_ptr<Nodes::DataBase> pop() = 0;
  virtual std::shared_ptr<Nodes::DataBase> top() = 0;
  virtual ~JsonStreamReader(){}

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
    stream->write(comm->uniqueId(), j, JsonStreamingEngineManager::id++);
  }

  virtual void syncId() {
    JsonStreamingEngineManager::id = commMapper.getNextId(comm, JsonStreamingEngineManager::id);
  }
  
  virtual void setComm(ICommunicator_ptr comm, bool syncIds) {
    
    setCommunicator(comm);
    commMapper.logComm(comm);
    long id = comm->uniqueId();
    

    json procList = commMapper.getCommWorldMap(comm);
    if (comm->Rank() == getRoot() && commids.find(id) == commids.end() ) {
      json nJson;

      nJson = json::object();
      nJson[JSD::name] = "comminfo";
      nJson[JSD::node] = JSN::commInfo;
      nJson[JSD::comm] = id;
      nJson[JSD::commList] = procList;
 
      stream->newComm(id, nJson); 
      commids.insert(id);
      
    }

    if (syncIds) {
      syncId();
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
                           bool internal, long uuid) override;

  void testFinishedCallBack(bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override;

  void unitTestFinishedCallBack(IUnitTest* tester) override;


  std::shared_ptr<Nodes::IRootNode> readFromFile(std::string file, long& idCounter) override;

  // IInternalOutputEngine interface
  std::string print() override;

};



}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // JsonStreamingEngineManager_H
