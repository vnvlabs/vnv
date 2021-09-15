
/**
  @file ActionStore.h
**/
#ifndef VV_WalkerSTORE_HEADER
#define VV_WalkerSTORE_HEADER

#include <map>
#include <string>

#include "base/stores/BaseStore.h"
#include "interfaces/IWalker.h"

namespace VnV {

class WalkerStore : public BaseStore {
 private:
  std::map<std::string, std::pair<VnV::walker_maker_ptr, nlohmann::json>>
      Walker_factory;

 public:
  WalkerStore() {}

  IWalker_ptr getWalker(std::string package, std::string name,
                        Nodes::IRootNode* rootNode, nlohmann::json& config);

  void addWalker(std::string packageName, std::string name,
                 nlohmann::json& schema, walker_maker_ptr m);

  static WalkerStore& instance();
};

}  // namespace VnV

#endif
