
#include "VnV.h"
#include "plugins/walkers/iter.h"
using namespace VnV::Nodes;

namespace {

class SequentialGraphIter : public VnV::Walkers::Iter {
 protected:
  long rootComm;
  std::list<IDN>::iterator indexIter;
  
  // Include a .
  bool includeProc(long streamId) {
    return streamId == rootComm || commMap->commIsChild(rootComm, streamId);
  }

  std::map<long, long> currentNodeForComm;
  std::map<long, std::set<long>> commSubs;

  std::set<long>& getCommSubs(long comm) {
    auto it = commSubs.find(comm);
    if (it != commSubs.end()) {
      return it->second;
    }

    commSubs[comm] = {};
    for (auto it : commSubs) {
      if (comm == it.first) continue;
      if (commMap->commIsChild(it.first, comm)) {
        it.second.insert(comm);
      }
    }
    currentNodeForComm[comm] = -1;
    return commSubs[comm];
  }

  void addEdges(long comm, long streamId, long nodeId, std::set<long>& edges) {
    bool intersect =
        comm == streamId || commMap->commsIntersect(streamId, comm);

    if (!intersect) {
      return;  // No intersection between the two comms.
    } else {
      // Iterate over the comms that intersect this comm. If they
      // contain this stream, they will set the edge.

      for (auto c : getCommSubs(comm)) {
        addEdges(c, streamId, nodeId, edges);
      }

      if (edges.size() == 0) {
        // None of the child comms to this comm wanted this node. So it
        // must be our node.
        long pn = currentNodeForComm[comm];

        if (pn > -1) {
          edges.insert(pn);
        }
      }

      if (streamId == comm) {
        currentNodeForComm[streamId] = nodeId;
      }
    }
  }

  std::set<long>& getEdges(long streamId, long nodeId, std::set<long>& edges) {
    addEdges(streamId, streamId, nodeId, edges);
    return edges;
  }

  // Get the next node id->vector of parents

 public:
  SequentialGraphIter(
      ICommMap* comm, long commId,
      std::map<long, std::list<IDN>>& n)
      : Iter(comm, n), rootComm(commId) {
    indexIter = niter->second.begin();
  }
  virtual bool next(std::tuple<long, node_type, long, std::set<long>>& res) {
    while (niter != nodes.end()) {
      while (indexIter != niter->second.end()) {
        if (includeProc(indexIter->streamId)) {
          std::get<0>(res) = indexIter->id;
          std::get<1>(res) = indexIter->type; 
          std::get<2>(res) = indexIter->duration;
          std::get<3>(res).clear();
          getEdges(indexIter->streamId, indexIter->id, std::get<3>(res));
          ++indexIter;
          return true;
        }
        ++indexIter;
      }
      ++niter;
    }

    return false;
  }


};

class RootNodeGraphWalk : public VnV::IWalker {

  IRootNode* rootNode;
  std::shared_ptr<SequentialGraphIter> procIter;
  std::tuple<long, node_type, long, std::set<long>> curr;


  virtual bool _next(VnV::Nodes::WalkerNode& node) override { 
    if (procIter->next(curr)) {
        node.item = rootNode->findById(std::get<0>(curr));
        node.type = std::get<1>(curr);
        node.time = std::get<2>(curr);
        node.edges = std::get<3>(curr);
        return true;
    } 

    node.item = NULL;
    node.type = node_type::DONE;
    node.edges.clear();
    return false;
  }

 public:
  RootNodeGraphWalk(IRootNode* root, long commId) : IWalker(root) {
    rootNode = root;
    procIter = std::make_shared<SequentialGraphIter>(
         root->getCommInfoNode()->getCommMap(),commId, root->getNodes());
    }


};

const char* getSchema() {
  return R"(
  {  
     "type": "object",
     "properties" : {
        "id"   : {"type" : "integer" }
     },
     "required" : ["id","comm","only"]
  })";
}

}

INJECTION_WALKER_S(VNVPACKAGENAME,graph, getSchema()) {
    long id = config["id"].get<long>();
    return new RootNodeGraphWalk(rootNode,id);
}


