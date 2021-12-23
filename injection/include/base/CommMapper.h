#ifndef COMMMAPPER_H
#define COMMMAPPER_H

#include <functional>
#include <set>

#include "exceptions.h"
#include "interfaces/ICommunicator.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

using VnV::ICommunicator_ptr;

class CommWrap {
 public:
  long id = 0;

  explicit CommWrap(long id_) : id(id_) {}

  std::map<long, std::shared_ptr<CommWrap>> children = {};
  std::map<long, std::shared_ptr<CommWrap>> parents = {};
  std::set<long> contents = {};

  void toJson1(json& j, std::set<long>& done) {
    if (j.find("nodes") == j.end()) {
      j["nodes"] = json::array();
      j["links"] = json::array();
    }

    if (done.find(id) == done.end()) {
      json jj = json::object();
      jj["id"] = std::to_string(id);
      jj["group"] = contents.size();
      json p = json::array();
      for (const auto& it : parents) {
        p.push_back(it.second->id);
      }
      jj["parents"] = p;
      if (contents.size() == 1) {
        jj["world-rank"] = (*contents.begin());
      }
      j["nodes"].push_back(jj);

      done.insert(id);

      for (auto& it : children) {
        json cj = json::object();
        cj["source"] = std::to_string(id);
        cj["target"] = std::to_string(it.second->id);
        cj["value"] = it.second->contents.size();
        j["links"].push_back(cj);
        it.second->toJson1(j, done);
      }
    }
  }

  nlohmann::json toJson() {
    nlohmann::json j;
    std::set<long> done;
    toJson1(j, done);
    return j;
  }

  void getCommChain(std::set<long>& result) {
    auto it = result.find(id);
    if (it != result.end()) {
      result.insert(id);
      for (auto& p : parents) p.second->getCommChain(result);
      for (auto& c : children) c.second->getCommChain(result);
    }
  }

  bool inCommChain(long comm) {
    std::set<long> visited;
    return inCommChain(comm, visited);
  }

  bool inCommChain(long comm, std::set<long>& visited) {
    if (comm == id) {
      return true;
    }
    auto it = visited.find(id);
    if (it == visited.end()) {
      visited.insert(id);
      for (auto& p : parents) {
        if (p.first == comm || p.second->inCommChain(comm, visited)) {
          return true;
        }
      }
      for (auto& c : children) {
        if (c.first == comm || c.second->inCommChain(comm, visited)) {
          return true;
        }
      }
    }
    return false;
  }

  std::set<long> getCommChain() {
    std::set<long> r;
    getCommChain(r);
    return r;
  }

  std::string print(int c = 0) { return toJson().dump(); }
};

typedef std::shared_ptr<CommWrap> CommWrap_ptr;
typedef std::map<long, CommWrap_ptr> CommMap;

class CommMapper {
 public:
  static long id;
  int root = 0;
  std::set<long> comms;
  std::set<long> rootComms;

  bool isNew(ICommunicator_ptr comm);

  // This should track the cores present in a communicator. This should
  // return a unique id to use for this communicator where the id is constant
  // for communicators with the same processors in them.
  void logComm(ICommunicator_ptr comm);

  json getCommWorldMap(ICommunicator_ptr comm, int root = 0) {
    std::vector<int> res(comm->Rank() == root ? comm->Size() : 0);
    int rank = comm->Rank();
    comm->Gather(&rank, 1, res.data(), sizeof(int), root);
    json nJson = res;
    return nJson;
  }

  long getNextId(ICommunicator_ptr comm, long myVal);

  std::set<CommWrap_ptr> gatherCommInformation(ICommunicator_ptr worldComm);

  json getCommJson(ICommunicator_ptr worldcomm);

  static void commsMapSetToMap(const CommWrap_ptr& ptr,
                               std::map<long, CommWrap_ptr>& comms) {
    auto it = comms.find(ptr->id);
    if (it == comms.end()) {
      comms.insert(std::make_pair(ptr->id, ptr));
      for (auto& ch : ptr->children) {
        commsMapSetToMap(ch.second, comms);
      }
    }
  }

  static std::map<long, CommWrap_ptr> convertToMap(
      std::set<CommWrap_ptr>& comms) {
    std::map<long, CommWrap_ptr> m;
    for (auto& it : comms) {
      commsMapSetToMap(it, m);
    }
    return m;
  }

  std::vector<long> listAllComms(ICommunicator_ptr sharedPtr);
};

enum class DataRelative {
  PARENT,
  CHILD,
  OLDERSIBLING,
  YOUNGERSIBLING,
  STRANGER
};

class InjectionPointInterface {
 public:
  virtual long startId() = 0;
  virtual long endId() = 0;
  virtual long comm() = 0;
  virtual bool injectionPoint() = 0;
  virtual std::vector<std::shared_ptr<InjectionPointInterface>> children();
};

class InjectionPointMerger {
 public:
  bool isRoot = false;
  InjectionPointInterface& main;
  long idstart = -1;
  long idstop = -1;
  long comm = -1;

  std::map<long, std::set<std::shared_ptr<InjectionPointMerger>>> children;
  std::set<long> chain;
  CommWrap_ptr commWrap;

  InjectionPointMerger(InjectionPointInterface& m,
                       std::map<long, std::shared_ptr<CommWrap>>& comms,
                       bool isR = false)
      : isRoot(isR), main(m) {
    if (!isRoot) {
      idstart = main.startId();
      idstop = main.endId();
      comm = main.comm();
      chain = comms.find(comm)->second->getCommChain();
    }

    auto cc = main.children();
    for (auto c = cc.begin(); c != cc.end();) {
      if ((*c)->injectionPoint()) {
        addChildInternal(std::make_shared<InjectionPointMerger>(**c, comms));
        c = cc.erase(c);
      } else {
        ++c;
      }
    }
  }

  void addChildInternal(std::shared_ptr<InjectionPointMerger> data) {
    auto it = children.find(data->idstart);
    if (it == children.end()) {
      children[data->idstart] = {data};
    } else {
      children[data->idstart].insert(data);
    }
  }

  DataRelative getRelation(std::shared_ptr<InjectionPointMerger> data) {
    bool chainContains = chain.find(data->comm) != chain.end();
    if (!isRoot && !chainContains) {
      return DataRelative::STRANGER;
    } else if (!isRoot && data->idstop < idstart)
      return DataRelative::OLDERSIBLING;
    else if (!isRoot && data->idstart > idstop)
      return DataRelative::YOUNGERSIBLING;
    else if (!isRoot && data->idstart < idstart && data->idstop > idstop) {
      // data->getRelation(this);// Add myself as a child to data.
      return DataRelative::PARENT;
    } else if (isRoot || (data->idstart > idstart && data->idstop < idstop)) {
      bool isNewChild = true;
      for (auto childStartId = children.begin();
           childStartId != children.end();) {
        for (auto child = childStartId->second.begin();
             child != childStartId->second.end();) {
          auto r = (*child)->getRelation(data);
          if (r == DataRelative::CHILD) {
            return DataRelative::CHILD;  // child so its handled --> Return .
          } else if (r == DataRelative::PARENT) {
            // The get realation call has already added child as a child data.
            data->getRelation(*child);
            child = childStartId->second.erase(child);  // erase this as child.
            isNewChild = true;
            // Can't return just yet, need to check other siblings are not
            // children of it.
          } else {
            child++;  // Its a sibling, so just continue and add at bottom.
          }
        }
        if (childStartId->second.size() == 0) {
          childStartId = children.erase(childStartId);
        } else {
          childStartId++;
        }
      }
      // If we reach here, then its my child.
      addChildInternal(data);
      return DataRelative::CHILD;
    }
    return DataRelative::STRANGER;
  }

  static void join(
      std::shared_ptr<InjectionPointMerger>& datastruct, long commId,
      std::map<long, CommWrap_ptr> comms, std::string outfile,
      std::set<long>& done,
      std::function<std::shared_ptr<InjectionPointInterface>(long)>& parse) {
    // Don't add comms twice.
    auto comm = comms.find(commId)->second;
    if (done.find(commId) != done.end())
      return;
    else
      done.insert(commId);

    // Parse the file into a json object.
    std::shared_ptr<InjectionPointInterface> thisMerger = parse(comm->id);

    if (datastruct == nullptr) {
      datastruct.reset(new InjectionPointMerger(*thisMerger, comms, true));
    } else {
      // Get a root structure going.
      for (const auto& it : thisMerger->children()) {
        if (it->injectionPoint()) {
          auto d = std::make_shared<InjectionPointMerger>(*it, comms);
          datastruct->getRelation(d);
        } else {
          // Just add it to the main structs children.
          datastruct->main.children().push_back(it);
        }
      }
    }
    //
  }

  static std::shared_ptr<InjectionPointMerger> join(
      std::string outputfile, std::set<CommWrap_ptr>& comms,
      std::function<std::shared_ptr<InjectionPointInterface>(long)>& parse) {
    
    INJECTION_ASSERT(comms.size() != 1, "Invalid Comms Object. Size should be one but it was %d", comms.size());
    
    std::map<long, CommWrap_ptr> commsMap = CommMapper::convertToMap(comms);
    std::shared_ptr<InjectionPointMerger> dstruct = nullptr;
    std::set<long> done;
    for (auto it : commsMap) {
      join(dstruct, it.second->id, commsMap, outputfile, done, parse);
    }
    return dstruct;
  }
};

class DynamicCommMap {
 public:
  CommMap map;
  int worldsize;
  DynamicCommMap(int worldSize);

  void append(long commId, std::vector<int> procs);

  CommWrap_ptr getRootCommunicator();

  bool searchNodeInMainNodeCommChain(long mainNode, long searchNode);
};

}  // namespace VnV

#endif