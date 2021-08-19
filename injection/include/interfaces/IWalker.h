#ifndef WALKER_HEADER_VNV
#define WALKER_HEADER_VNV

#include "interfaces/Nodes.h"

namespace VnV {

class WalkerNode {
 public:
  Nodes::DataBase* item;
  Nodes::node_type type;
  std::set<long> edges;
};

class IWalker {
 Nodes::IRootNode* rootNode;
public:    
  IWalker(Nodes::IRootNode* root) : rootNode(root) {
     rootNode->registerWalkerCallback(this);
  }
  
  virtual void callback(long index, std::list<std::tuple<long,long,Nodes::node_type>>::iterator iter){}
  
  virtual bool next(WalkerNode& item) = 0;
  
  virtual ~IWalker(){
      rootNode->deregisterWalkerCallback(this);
  };
};

typedef IWalker* (*walker_maker_ptr)(Nodes::IRootNode* rootNode, nlohmann::json& config);
typedef std::shared_ptr<IWalker> IWalker_ptr;

void registerWalker(std::string package, std::string name, std::string schema,
                    VnV::walker_maker_ptr m);

}  // namespace VnV


#define INJECTION_WALKER_S(PNAME, name, schema)                         \
                                                                        \
  namespace VnV {                                                       \
  namespace PNAME {                                                     \
  namespace Walkers {                                                   \
  IWalker* declare_##name(IRootNode* rootNode, nlohmann::json& config); \
                                                                        \
  void register_##name() {                                              \
    VnV::registerWalker(VNV_STR(PNAME), #name, schema, &declare_##name); \
  }                                                                     \
  }                                                                     \
  }                                                                     \
  }                                                                     \
  VnV::IWalker* VnV::PNAME::Walkers::declare_##name(IRootNode* rootNode, \
                                                   nlohmann::json& config)

#define INJECTION_WALKER(PNAME, name) \
  INJECTION_WALKER_S(PNAME, name, R"({"type":"object"})")

#define DECLAREWALKER(PNAME, name) \
  namespace VnV {                  \
  namespace PNAME {                \
  namespace Walkers {              \
  void register_##name();          \
  }                                \
  }                                \
  }

#define REGISTERWALKER(PNAME, name) VnV::PNAME::Walkers::register_##name();

#endif