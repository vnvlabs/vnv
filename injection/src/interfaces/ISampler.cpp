#include "interfaces/ISampler.h"
#include "base/stores/SamplerStore.h"

void VnV::registerSampler(
    std::string packageName, std::string name,
    std::string schema,
    VnV::sampler_ptr ptr) {
    VnV::SamplerStore::instance().addSampler(packageName, name, schema, ptr);
}
