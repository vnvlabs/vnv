
/**
  @file ActionStore.h
**/
#ifndef VV_SAMPLERSTORE_HEADER
#define VV_SAMPLERSTORE_HEADER

#include <map>
#include <string>

#include "interfaces/ISampler.h"
#include "base/stores/BaseStore.h"


namespace VnV {


class SamplerStore : public BaseStore {

private:
  std::map<std::string, ISampler_ptr> samplers;
  std::map<std::string, std::pair<sampler_ptr, std::string>> sampler_factory;
  

 public:
  SamplerStore(){}

  ISampler_ptr getSamplerForInjectionPoint(std::string ipPackage, std::string ipName);

  void addSampler(std::string packageName, std::string name, std::string schema, sampler_ptr m);

  bool createSampler(const SamplerConfig& config);

  void print();

  static SamplerStore& instance();

};

}  // namespace VnV

#endif
