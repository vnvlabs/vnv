#ifndef IPIPELINE_H
#define IPIPELINE_H

#include "json-schema.hpp"
using nlohmann::json; 
namespace VnV {


class IPipeline {

public:
   virtual std::string schema() { return json::object(); }
   virtual std::string generatePipeline(const json& config) = 0;
   IPipeline() {}
   virtual ~IPipeline() {}

};
typedef std::shared_ptr<IPipeline> IPipeline_ptr;
typedef IPipeline* (*pipeline_ptr)();

void registerPipeline(std::string packageName, std::string name,
                       VnV::pipeline_ptr r);

}  // namespace VnV

#define INJECTION_PIPELINE(PNAME, name, SCHEMA)                     \
  namespace VnV {                                                   \
  namespace PNAME {                                                 \
  namespace Pipelines {                                             \
  class name : public IPipeline {                                   \
  public:                                                           \
    std::string schema() override { return SCHEMA; }                \
    std::string generatePipeline(const json& config) override;      \
  };                                                                \
                                                                    \
  VnV::IPipeline* declare_##name() { return new name(); }           \
  void register_##name() {                                          \
    VnV::registerPipeline(VNV_STR(PNAME), #name, &declare_##name);  \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  std::string VnV::PNAME::Pipelines::name::generatePipeline(const json& config)


#define DECLAREPIPELINE(PNAME, name) \
  namespace VnV {                   \
  namespace PNAME {                 \
  namespace Pipelines {              \
  void register_##name();           \
  }                                 \
  }                                 \
  }

#define REGISTERPIPELINE(PNAME, name) VnV::PNAME::Pipelines::register_##name();



#endif

