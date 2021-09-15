#ifndef WALKER_HEADER_VNV
#define WALKER_HEADER_VNV

#include "interfaces/Nodes.h"

namespace VnV {

class IWalker {
  Nodes::IRootNode* rootNode;

  virtual bool _next(Nodes::WalkerNode& item) = 0;

 public:
  IWalker(Nodes::IRootNode* root) : rootNode(root) {
    rootNode->registerWalkerCallback(this);
  }

  virtual void callback(long index, std::list<Nodes::IDN>::iterator iter) {}

  // This lets up wrap the next call at the walker level. I put this in so
  // we could lock the file before walking, but ended up implementing it
  // another way. Could be useful at a later date so am leaving the wrapper
  // around the pure virtual _next function in place.
  virtual bool next(Nodes::WalkerNode& item) { return _next(item); }

  virtual ~IWalker() { rootNode->deregisterWalkerCallback(this); };
};

typedef IWalker* (*walker_maker_ptr)(Nodes::IRootNode* rootNode,
                                     nlohmann::json& config);
typedef std::shared_ptr<IWalker> IWalker_ptr;

void registerWalker(std::string package, std::string name, std::string schema,
                    VnV::walker_maker_ptr m);

}  // namespace VnV

#define INJECTION_WALKER_S(PNAME, name, schema)                          \
                                                                         \
  namespace VnV {                                                        \
  namespace PNAME {                                                      \
  namespace Walkers {                                                    \
  IWalker* declare_##name(IRootNode* rootNode, nlohmann::json& config);  \
                                                                         \
  void register_##name() {                                               \
    VnV::registerWalker(VNV_STR(PNAME), #name, schema, &declare_##name); \
  }                                                                      \
  }                                                                      \
  }                                                                      \
  }                                                                      \
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