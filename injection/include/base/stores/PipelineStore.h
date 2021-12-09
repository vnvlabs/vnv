
/**
  @file ActionStore.h
**/
#ifndef VV_PIPELINESTORE_HEADER
#define VV_PIPELINESTORE_HEADER

#include <map>
#include <string>

#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IPipeline.h"

namespace VnV {

class PipelineStore : public BaseStore {
  std::map<std::string, pipeline_ptr> pipeline_factory;

 public:
  PipelineStore() {}

  std::string getPipeline(std::string package, std::string name, const json& config);

  void addPipeline(std::string packageName, std::string name, pipeline_ptr m);

  static PipelineStore& instance();
};

}  // namespace VnV

#endif
