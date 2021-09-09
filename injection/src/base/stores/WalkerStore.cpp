#include "base/stores/WalkerStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/exceptions.h"

namespace VnV {

BaseStoreInstance(WalkerStore)

    void WalkerStore::addWalker(std::string packageName, std::string name,
                                nlohmann::json& schema, walker_maker_ptr m) {
 
   std::cout << packageName + ":" + name << std::endl;
  Walker_factory[packageName + ":" + name] = std::make_pair(m, schema);
}

IWalker_ptr WalkerStore::getWalker(std::string package, std::string name,
                                   Nodes::IRootNode* rootNode,
                                   nlohmann::json& config) {
  
  
  
  auto it = Walker_factory.find(package + ":" + name);
  if (it != Walker_factory.end()) {
    if (JsonUtilities::validate(config, it->second.second)) {
       std::shared_ptr<IWalker> ptr;
       ptr.reset(it->second.first(rootNode, config));
       return ptr;
    }
  }
  return nullptr;
}

void registerWalker(std::string package, std::string name, std::string schema,
                    VnV::walker_maker_ptr m) {

  nlohmann::json s = nlohmann::json::parse(schema);
  WalkerStore::instance().addWalker(package, name, s, m);
}

}  // namespace VnV
