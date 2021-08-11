
#include "VnV.h"
#include "plugins/walkers/iter.h"
using namespace VnV::Nodes;

namespace {

class BasicNodeIter : public VnV::IWalker {
 protected:
  IRootNode *root;
  std::map<long, std::list<std::tuple<long, long, VnV::Nodes::node_type>>>::iterator niter;
  std::list<std::tuple<long,long,node_type>>::iterator indexIter;
   

 public:
  BasicNodeIter(IRootNode* rootNode) : IWalker(rootNode), root(rootNode) {
    niter = root->getNodes().begin();
    if (niter != root->getNodes().end() ) {
       indexIter = niter->second.begin();
    }
  }
    
  bool next(VnV::WalkerNode& node) override {
     while (niter != root->getNodes().end()) {
       while (indexIter!= niter->second.end()) {
         node.item = root->findById(std::get<1>(*indexIter));
         node.type = std::get<2>(*indexIter); 
         node.edges.clear();
         ++indexIter;
         return true;
       }
       ++niter;
     }
     node.item = NULL;
     node.type = node_type::DONE;
     node.edges.clear();
     return false;
  }
  
  // Override the callback function to be informed when new nodes are added to 
  // the root node. 
  virtual void callback(long index, std::list<std::tuple<long,long,node_type>>::iterator a) override {
      std::cout << "New Node Available " << std::endl; 
      
  }


};


}

/**
  * Iterate through all the nodes ordered by 
  * index. Indices are monotonically non increasing. So, you 
  * may see two indicies with the same value. In this case, these 
  * nodes are gauranteed to be coming from two independent communicators.
**/

INJECTION_WALKER(VNVPACKAGENAME,basic) {
    return new BasicNodeIter(rootNode);
}
