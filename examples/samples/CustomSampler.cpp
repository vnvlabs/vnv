
/** Include the VnV include file **/
#include "VnV.h"

class sampleRunner {
 public:
  sampleRunner() {}

  bool run(VnV::InjectionPointType type, std::string stageId) { return true; }
};

/**
 * Sampler that always says run.
 *
 */
INJECTION_SAMPLER_R(Samples, identitySampler, sampleRunner) {
  return runner->run(type, stageId);
}
