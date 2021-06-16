
/**
  @file InjectionPointStore.h
**/

#ifndef INJECTIONPLUGSTORE_H
#define INJECTIONPLUGSTORE_H

#include "base/stores/InjectionPointStore.h"
#include "interfaces/IPlug.h"
#include "base/points/PlugPoint.h"
using nlohmann::json;

namespace VnV {

class PlugSpec : public InjectionPointSpec {
public:   
      PlugSpec(std::string package, std::string name, json& spec) : InjectionPointSpec(package,name,spec) {}
};


class InjectionPlugConfig : public InjectionPointConfig {
public:

    std::shared_ptr<PlugConfig> plug = nullptr;

    InjectionPlugConfig(std::string package,
                         std::string id,
                         bool runInternal_,
                         std::vector<TestConfig>& tests_,
                         std::shared_ptr<PlugConfig> &plug_) :
                         InjectionPointConfig(package,id,runInternal_,tests_), plug(plug_) {}
};


class PlugStore {
 private:

  std::map<std::string, InjectionPlugConfig>  plugs; /**< The stored configurations */
  std::map<std::string, PlugSpec> registeredPlugs;


  PlugStore();

  std::shared_ptr<PlugPoint> newPlug(std::string packageName,
                                                    std::string name,
                                                    NTV& in_args, NTV& out_args);


public:

  void registerPlug(std::string packageName, std::string id, json& jsonObject);


  std::shared_ptr<PlugPoint> getNewPlug( std::string package,
                                                       std::string name,
                                                       NTV& in_args, NTV& out_args);

 
  void addPlug(std::string package,
                         std::string name,
                         bool runInternal,
                         std::vector<TestConfig> &tests,
                         std::shared_ptr<PlugConfig> &plug
                        );

  // Register Injection point. JsonStr must be json that validates against the
  // injection point schema OR an array of objects that individually validate
  // against that same schema.
  void registerPlug(std::string packageName, std::string name, std::string json_str);
 
  /**
   * @brief getInjectionPointStore
   * @return The InjectionPointStore
   *
   * static initializer for the InjectionPointStore. This function creates a
   * static store on first call, returning that object on each subsequent call.
   */
  static PlugStore& getPlugStore();

};  // end InjectionPointStore

}  // namespace VnV

#endif  // INJECTIONPOINTSTORE_H
