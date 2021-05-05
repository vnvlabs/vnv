#ifndef COMMMAPPER_H
#define COMMMAPPER_H

#include "interfaces/ICommunicator.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

using VnV::Communication::ICommunicator_ptr;

class CommWrap {
public:
    long id = 0;

    explicit CommWrap(long id_) : id(id_) {}

    std::map<long,std::shared_ptr<CommWrap>> children = {};
    std::map<long,std::shared_ptr<CommWrap>> parents = {};
    std::set<long> contents = {};

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
         for (const auto &it : parents) {
            p.push_back(it.second->id);
         }
         jj["parents"] = p;
         if (contents.size()==1) {
           jj["world-rank"] = (*contents.begin());
         }
         j["nodes"].push_back(jj);

         done.insert(id);

         for (auto &it : children) {
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
          for (auto &p : parents) p.second->getCommChain(result);
          for (auto &c : children) c.second->getCommChain(result);
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
   
   bool isNew(Communication::ICommunicator_ptr comm);

   //This should track the cores present in a communicator. This should
   //return a unique id to use for this communicator where the id is constant
   // for communicators with the same processors in them.
   void logComm(Communication::ICommunicator_ptr comm);
   
   long getNextId(Communication::ICommunicator_ptr comm, long myVal);
   
   std::set<CommWrap_ptr> gatherCommInformation(ICommunicator_ptr worldComm);
   
   json getCommJson(ICommunicator_ptr worldcomm);

   static void commsMapSetToMap(const CommWrap_ptr &ptr, std::map<long,CommWrap_ptr> &comms) {
     auto it = comms.find(ptr->id);
     if (it == comms.end()) {
       comms.insert(std::make_pair(ptr->id,ptr));
       for (auto &ch : ptr->children) {
         commsMapSetToMap(ch.second, comms);
       }

     }
   }

   static std::map<long, CommWrap_ptr> convertToMap(std::set<CommWrap_ptr> &comms) {
     std::map<long,CommWrap_ptr > m;
     for (auto &it : comms) {
       commsMapSetToMap(it, m);
     }
     return m;
   }


};

}

#endif