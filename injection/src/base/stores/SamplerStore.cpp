#include "base/stores/SamplerStore.h"

#include <iostream>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/Runtime.h"

namespace VnV {

BaseStoreInstance(SamplerStore)

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

nlohmann::json SamplerStore::schema() {
    nlohmann::json oneof = json::array();
    for (auto &it :  sampler_factory) {
      std::vector<std::string> ss;
      StringUtils::StringSplit(it.first,":",ss);

      nlohmann::json properties = json::object();
      nlohmann::json req = json::array();

      nlohmann::json package = json::object();
      package["const"] = ss[0];
      
      json name = json::object();
      name["const"] = ss[1];
      
      properties["package"] = package;
      properties["name"] = name;
      properties["config"] = it.second.second;
      
      json p = json::object();
      p["type"] = "object";
      p["properties"] = properties;
      p["required"] = R"(["name","package"])"_json;
      oneof.push_back(p);  
    }
    
      if (oneof.size() > 0 ) {
      nlohmann::json ret = json::object();
      ret["oneOf"] = oneof;
      return ret;
    } else {
      return R"({"const" : false})"_json;
    }
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

}  // namespace VnV
