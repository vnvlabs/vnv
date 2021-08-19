#ifndef VNV_ISAMPLER_HEADER
#define VNV_ISAMPLER_HEADER

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "interfaces/IOutputEngine.h"

namespace VnV {

class SamplerConfig {
 public:
  bool active = false;
  std::string name = "";
  std::string package = "";
  std::string ipName = "";
  std::string ipPackage = "";
  nlohmann::json config = json::object();

  SamplerConfig(const InjectionPointInfo& info) {
    if (!info.sampler.name.empty()) {
      active = true;
      name = info.sampler.name;
      package = info.sampler.package;
      ipName = info.name;
      ipPackage = info.package;
      config = info.sampler.config;
    } else {
      active = false;
    }
  }
};

class ISampler;
typedef std::shared_ptr<ISampler> ISampler_ptr;
typedef std::vector<ISampler_ptr> ISampler_vec;

class ISampler {
 private:
 protected:
  nlohmann::json config;

 public:
  ISampler() {}
  virtual ~ISampler() {}

  virtual bool sample(VnV::InjectionPointType stage, std::string stageId) {
    return true;
  }
};

typedef ISampler* (*sampler_ptr)(nlohmann::json&);

void registerSampler(std::string packageName, std::string name,
                     std::string schema, VnV::sampler_ptr r);

template <typename Runner> class Sampler_T : public ISampler {
 public:
  nlohmann::json config;

  std::shared_ptr<Runner> runner;

  Sampler_T(nlohmann::json& config_)
      : ISampler(), config(config_), runner(new Runner()) {}
};

}  // namespace VnV

#define INJECTION_SAMPLER_RS(PNAME, name, Runner, schema)                \
  namespace VnV {                                                        \
  namespace PNAME {                                                      \
  namespace Samplers {                                                   \
  class name : public Sampler_T<Runner> {                                \
    public : name(nlohmann::json& config)                                \
        : Sampler_T(config){};                                           \
    virtual bool sample(VnV::InjectionPointType type,                    \
                        std::string stageId) override;                   \
  };                                                                     \
                                                                         \
  VnV::ISampler* declare_##name(nlohmann::json& config) {                \
    return new name(config);                                             \
  }                                                                      \
  void register_##name() {                                               \
    VnV::registerSampler(VNV_STR(PNAME), #name, schema, &declare_##name); \
  }                                                                      \
  }                                                                      \
  }                                                                      \
  }                                                                      \
  bool VnV::PNAME::Samplers::name::sample(VnV::InjectionPointType type,  \
                                          std::string stageId)

#define INJECTION_SAMPLER_S(PNAME, name, schema) \
  INJECTION_SAMPLER_RS(PNAME, name, int, schema)

#define INJECTION_SAMPLER_R(PNAME, name, Runner) \
  INJECTION_SAMPLER_RS(PNAME, name, Runner, R"({"type":"object"})")

#define INJECTION_SAMPLER(PNAME, name) \
  INJECTION_SAMPLER_S(PNAME, name, R"({"type":"object"})")

#define DECLARESAMPLER(PNAME, name) \
  namespace VnV {                   \
  namespace PNAME {                 \
  namespace Samplers {              \
  void register_##name();           \
  }                                 \
  }                                 \
  }

#define REGISTERSAMPLER(PNAME, name) VnV::PNAME::Samplers::register_##name();

#endif