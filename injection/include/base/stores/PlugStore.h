
/**
  @file InjectionPointStore.h
**/

#ifndef INJECTIONPLUGSTORE_H
#define INJECTIONPLUGSTORE_H

#include "base/points/PlugPoint.h"
#include "base/stores/BaseStore.h"
#include "base/stores/InjectionPointStore.h"
#include "interfaces/IPlug.h"

using nlohmann::json;

namespace VnV {

class PlugSpec : public InjectionPointSpec {
 public:
  PlugSpec(std::string package, std::string name, json& spec)
      : InjectionPointSpec(package, name, spec) {}
};

class InjectionPlugConfig : public InjectionPointConfig {
 public:
  std::shared_ptr<PlugConfig> plug = nullptr;

  InjectionPlugConfig(std::string package, std::string id, bool runInternal_, json& templateName,
                      std::vector<TestConfig>& tests_, 
                      std::shared_ptr<PlugConfig>& plug_)
      : InjectionPointConfig(package, id, runInternal_, templateName, tests_), plug(plug_) {}
};

class PlugStore : public BaseStore {
 private:
  std::map<std::string, InjectionPlugConfig>
      plugs; /**< The stored configurations */
  std::map<std::string, PlugSpec> registeredPlugs;

  std::shared_ptr<PlugPoint> newPlug(std::string packageName, std::string name,
                                     const VnV::TemplateCallback& templateCallback, 
                                     NTV& in_args, NTV& out_args);

 public:
  PlugStore();

  void registerPlug(std::string packageName, std::string id, json& jsonObject);

  std::shared_ptr<PlugPoint> getNewPlug(std::string package, std::string name,
                                        const VnV::TemplateCallback& templateCallback, 
                                        NTV& in_args, NTV& out_args);

  nlohmann::json schema();

  void addPlug(std::string package, std::string name, bool runInternal, json& templateName, 
               std::vector<TestConfig>& tests,
               std::shared_ptr<PlugConfig>& plug);

  // Register Injection point. JsonStr must be json that validates against the
  // injection point schema OR an array of objects that individually validate
  // against that same schema.
  void registerPlug(std::string packageName, std::string name,
                    std::string json_str);
  static PlugStore& instance();

};  // end InjectionPointStore

}  // namespace VnV

#endif  // INJECTIONPOINTSTORE_H
