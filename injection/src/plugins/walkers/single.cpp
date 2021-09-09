
#include "VnV.h"
#include "plugins/walkers/iter.h"
using namespace VnV::Nodes;

namespace {


class ProcIter : public VnV::Walkers::Iter {
 protected:
  long searchProc;

  // Include any comms that contain this processor.
  virtual bool procContainedIn(long streamId) {
    return commContainsProc(streamId, searchProc);
  }

 public:
  ProcIter(ICommMap* comm, long proc,
           std::map<long, std::list<IDN>>& n)
      : Iter(comm, n), searchProc(proc){};

  virtual bool next(IDN& res) {
    while (niter != nodes.end()) {
      for (auto& it : niter->second) {
        if (procContainedIn(it.streamId)) {
          res.duration = it.duration;
          res.id = it.id;
          res.streamId = it.streamId;
          res.type = it.type;
          ++niter;
          return true;
        }
        ++niter;
      }
    }
    return false;
  }
};



class CommIter : public ProcIter {
 protected:
  // Include any comms that contain me in entirty
  virtual bool procContainedIn(long streamId) override {
    return parentContainsChild(streamId, searchProc);
  }

 public:
  CommIter(ICommMap* comm, long commId,
           std::map<long, std::list<IDN>>& n)
      : ProcIter(comm, commId, n) {}
};


class OnlyCommIter : public ProcIter {
 protected:
  // Include any comms that contain me in entirty
  virtual bool procContainedIn(long streamId) override {
    return streamId == searchProc;
  }

 public:
  OnlyCommIter(ICommMap* comm, long commId,
               std::map<long, std::list<IDN>>& n)
      : ProcIter(comm, commId, n) {}
};


class OnlyProcIter : public ProcIter {
 protected:
  // Include any comms that contain me in entirty
  virtual bool procContainedIn(long streamId) override {
    return commMap->commIsSelf(streamId, searchProc);
  }

 public:
  OnlyProcIter(ICommMap* comm, long commId,
               std::map<long, std::list<IDN>>& n)
      : ProcIter(comm, commId, n) {}
};


class RootNodeProcWalk : public VnV::IWalker {

  IRootNode* rootNode;
  std::shared_ptr<ProcIter> procIter;
  IDN curr;

 virtual bool _next(VnV::Nodes::WalkerNode& node) override { 
    
    if (procIter->next(curr)) {
        node.item = rootNode->findById(curr.id);
        node.type = curr.type;
        node.time = curr.duration;
        node.edges.clear();
        return true;
    } 

    node.item = NULL;
    node.type = node_type::DONE;
    return false;

  }

 public:
  RootNodeProcWalk(IRootNode* root, long processor, bool only, bool comm) : IWalker(root) {
    rootNode = root;
    if (!comm) {
      if (only) {
        procIter = std::make_shared<OnlyProcIter>(
            root->getCommInfoNode()->getCommMap(), processor, root->getNodes());
      } else {
        procIter = std::make_shared<ProcIter>(root->getCommInfoNode()->getCommMap(),
                                              processor, root->getNodes());
      }
    } else {
      if (only) {
        procIter = std::make_shared<OnlyCommIter>(
            root->getCommInfoNode()->getCommMap(), processor, root->getNodes());
      } else {
        procIter = std::make_shared<CommIter>(root->getCommInfoNode()->getCommMap(),
                                              processor, root->getNodes());
      }
    }
  }

 

};

const char* getSchema() {
  return R"(
  {  
     "type": "object",
     "properties" : {
        "only" : {"type" : "boolean" },
        "comm" : {"type" : "boolean" },
        "id"   : {"type" : "integer" }
     },
     "required" : ["id","comm","only"]
  })";
}

}

INJECTION_WALKER_S(VNVPACKAGENAME,proc, getSchema()) {
    std::cout << "SDFSDFS" << std::endl;
    long proc = config["id"].get<long>();
    bool only = config["only"].get<bool>();
    bool comm = config["comm"].get<bool>();
    return new RootNodeProcWalk(rootNode,proc, only, comm);
}









