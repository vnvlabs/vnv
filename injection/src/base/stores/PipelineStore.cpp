#include "base/stores/PipelineStore.h"
#include "base/Runtime.h"
#include "base/exceptions.h"
#include "base/Utilities.h"
#include <iostream>


namespace VnV {

BaseStoreInstance(PipelineStore)

namespace {
 std::string getKey(std::string packageName, std::string name) {
    return packageName + ":" + name;
  }
}

void PipelineStore::addPipeline(std::string packageName, std::string name,
                                  pipeline_ptr m) {
  pipeline_factory.insert(std::make_pair(getKey(packageName, name), m));
}

std::string PipelineStore::getPipeline(std::string packageName,
                                          std::string name, const json& config) {

  auto it = pipeline_factory.find(getKey(packageName,name));
  if (it != pipeline_factory.end()) {
    IPipeline_ptr s(it->second());
    if (JsonUtilities::validate(config, json::parse(s->schema()))) {
      return s->generatePipeline(config);
    } else {
      throw VnV::VnVExceptionBase("Bad Config failed validation");
    }
  }
  throw VnV::VnVExceptionBase("Unknown Pipeline: %s:%s", packageName.c_str(), name.c_str());

}

}  // namespace VnV
