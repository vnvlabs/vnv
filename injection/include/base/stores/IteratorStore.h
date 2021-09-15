
/**
  @file InjectionPointStore.h
**/

#ifndef INJECTIONITERATORSTORE_H
#define INJECTIONITERATORSTORE_H

#include "base/points/IteratorPoint.h"
#include "base/stores/BaseStore.h"
#include "base/stores/InjectionPointStore.h"

using nlohmann::json;

namespace VnV {

class InjectionIteratorConfig : public InjectionPointConfig {
 public:
  std::vector<IteratorConfig> iterators;
  InjectionIteratorConfig(std::string package, std::string id,
                          bool runInternal_, std::vector<TestConfig>& tests_,
                          std::vector<IteratorConfig>& iterators_)
      : InjectionPointConfig(package, id, runInternal_, tests_),
        iterators(iterators_) {}
};

class IteratorStore : public BaseStore {
 private:
  std::map<std::string, InjectionIteratorConfig>
      iterators; /**< The stored configurations */
  std::map<std::string, InjectionPointSpec> registeredIterators;

  std::shared_ptr<IterationPoint> newIterator(std::string packageName,
                                              std::string name, int once_,
                                              NTV& in_args, NTV& out_args);

 public:
  IteratorStore();

  void registerIterator(std::string packageName, std::string id,
                        json& jsonObject);

  std::shared_ptr<IterationPoint> getNewIterator(std::string package,
                                                 std::string name, int once,
                                                 NTV& in_args, NTV& out_args);

  void addIterator(std::string package, std::string name, bool runInternal,
                   std::vector<TestConfig>& tests,
                   std::vector<IteratorConfig>& iterators);

  // Register Injection point. JsonStr must be json that validates against the
  // injection point schema OR an array of objects that individually validate
  // against that same schema.
  void registerIterator(std::string packageName, std::string name,
                        std::string json_str);

  static IteratorStore& instance();

  nlohmann::json schema();

};  // end InjectionPointStore

}  // namespace VnV

#endif  // INJECTIONPOINTSTORE_H
