#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include <iostream>

#include "interfaces/IOutputEngine.h"
#include "interfaces/ICommunicator.h"

#include <set>
#include <chrono>

namespace VnV {

class CommWrap {
public:
    long id = 0;

    CommWrap(long id_) : id(id_) {}

    std::map<long,std::shared_ptr<CommWrap>> children = {};
    std::map<long,std::shared_ptr<CommWrap>> parents = {};
    std::set<long> contents = {};

    void toJson(json &j) {
        j["name"] = std::to_string(id);
        j["value"] = contents.size();

        if (children.size() > 0 ) {
           json c = json::array();
           for (auto it : children) {
               c.push_back(it.second->toJson());
           }
           j["children"] = c;
        }`
    }

    void toJson1(json &j, std::set<long> &done) {
      if (j.find("nodes") == j.end()) {
        j["nodes"] = json::array();
        j["links"] = json::array();
      }
      if (done.find(id) == done.end()) {

          json jj = json::object();
         jj["id"] = std::to_string(id);
         jj["group"] = contents.size();
         json p = json::array();
         for (auto it : parents) {
            p.push_back(it.second->id);
         }
         jj["parents"] = p;
         if (contents.size()==1) {
           jj["world-rank"] = (*contents.begin());
         }
         j["nodes"].push_back(jj);

         done.insert(id);

         for (auto it : children) {
            json cj = json::object();
            cj["source"] = std::to_string(id);
            cj["target"] = std::to_string(it.second->id);
            cj["value"] = it.second->contents.size();
            j["links"].push_back(cj);
            it.second->toJson1(j,done);
          }
      }
    }

    nlohmann::json toJson() {
       nlohmann::json j;
       std::set<long> done;
       toJson1(j,done);
       return j;
    }

    void getCommChain(std::set<long> &result) {
       auto it = result.find(id);
       if ( it != result.end()) {
          result.insert(id);
          for (auto p : parents) p.second->getCommChain(result);
          for (auto c : children) c.second->getCommChain(result);
       }
    }

    std::set<long> getCommChain() {
       std::set<long> r;
       getCommChain(r);
       return r;
    }

    std::string print(int c = 0) {
        return toJson().dump();
    }
};

typedef std::shared_ptr<CommWrap> CommWrap_ptr;
typedef std::map<long, CommWrap_ptr> CommMap;

class CommMapper {
public:
   static long id;
   int root = 0;
   std::set<long> comms;

   //This should track the cores present in a communicator. This should
   //return a unique id to use for this communicator where the id is constant
   // for communicators with the same processors in them.
   void logComm(Communication::ICommunicator_ptr comm);
   long getNextId(Communication::ICommunicator_ptr comm, long myVal);
   std::set<CommWrap_ptr> gatherCommInformation(ICommunicator_ptr worldComm);
   json getCommJson(ICommunicator_ptr worldcomm);
};


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
