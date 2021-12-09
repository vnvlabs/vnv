
#include "VnV.h"
#include "plugins/walkers/iter.h"
#include "streaming/Nodes.h"
using namespace VnV::Nodes;

namespace {

class BasicNodeIter : public VnV::IWalker {
 protected:
  IRootNode* root;
  std::map<long, std::list<IDN>>::iterator niter;
  std::list<IDN>::iterator indexIter;

  bool _next(VnV::Nodes::WalkerNode& node) override {
    while (niter != root->getNodes().end()) {
      while (indexIter != niter->second.end()) {
        node.item = root->findById(indexIter->id);
        node.type = indexIter->type;
        node.time = indexIter->duration;
        node.edges.clear();
        ++indexIter;
        return true;
      }

      ++niter;
      if (niter != root->getNodes().end()) {
        indexIter = niter->second.begin();
      }
    }

    node.item = NULL;
    node.type = node_type::DONE;
    node.edges.clear();
    return false;
  }

 public:
  BasicNodeIter(IRootNode* rootNode) : IWalker(rootNode), root(rootNode) {
    niter = root->getNodes().begin();
    if (niter != root->getNodes().end()) {
      indexIter = niter->second.begin();
    }
  }

};

}  // namespace

/**
 * Iterate through all the nodes ordered by
 * index. Indices are monotonically non increasing. So, you
 * may see two indicies with the same value. In this case, these
 * nodes are gauranteed to be coming from two independent communicators.
 **/

INJECTION_WALKER(VNVPACKAGENAME, basic) { return new BasicNodeIter(rootNode); }
