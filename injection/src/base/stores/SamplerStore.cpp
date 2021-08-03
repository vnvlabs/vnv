#include "base/stores/SamplerStore.h"

#include <iostream>

#include "base/Utilities.h"
#include "base/exceptions.h"

namespace VnV {

void SamplerStore::addSampler(std::string packageName, std::string name, std::string schema,
                              sampler_ptr m) {
  sampler_factory[packageName + ":" + name] = std::make_pair(m, schema);
}


ISampler_ptr SamplerStore::getSamplerForInjectionPoint(std::string ipPackage, std::string ipName) {
  auto it = samplers.find(ipPackage + ":" + ipName);
  if (it != samplers.end()) {
    return it->second;
  }  
  return nullptr;
}

bool SamplerStore::createSampler(const SamplerConfig& config) {
  
  auto it = sampler_factory.find(config.package + ":" + config.name);
  
  if (it != sampler_factory.end()) {
    
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(it->second.second);
    try {
      validator.validate(config.config);
      json j = config.config; // const correct fail :(
      ISampler_ptr s(it->second.first(j));
      samplers[config.ipPackage +":"+config.ipName] = s;
      return true;
    } catch (std::exception e) {
      return false;
    }
  }
  return false;
}

SamplerStore& SamplerStore::instance() {
  static SamplerStore store;
  return store;
}

}  // namespace VnV
