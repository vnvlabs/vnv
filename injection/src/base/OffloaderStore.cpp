

#include "base/OffloaderStore.hpp"

using namespace VnV;

OffloaderStore::OffloaderStore(){}

void OffloaderStore::registerOffloader(std::string name, offloader_register_ptr *offload_ptr) {
  registeredOffloaders.insert(std::make_pair(name, offload_ptr ));
}

std::shared_ptr<IOffloader> OffloaderStore::getOffloader() {
  return manager;
}

std::shared_ptr<IOffloader> OffloaderStore::setOffloader(std::string key, nlohmann::json &config) {
  auto it = registeredOffloaders.find(key);
  if (it != registeredOffloaders.end()) {
      manager.reset((it->second)());
      manager->config(config);
    }
  return manager;
}

OffloaderStore &OffloaderStore::get(){
  static OffloaderStore engine;
  return engine;
}
