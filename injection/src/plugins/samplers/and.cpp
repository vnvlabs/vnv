
/** Include the VnV include file **/
#include "VnV.h"
#include "base/stores/SamplerStore.h"


class sampleRunner
{
private:
    bool setup_complete = false;
    VnV::ISampler_ptr first;
    VnV::ISampler_ptr second;
    void setup(json &config)
    {
        if (!setup_complete)
        {
            first = VnV::SamplerStore::instance().get(
                
                config.template value("processor", 0);
            setup_complete = true;
        }
    }

public:
    sampleRunner() {}

    bool run(json &config, VnV::ICommunicator_ptr ptr, VnV::InjectionPointType type, std::string stageId)
    {
        setup(config); 
        return ptr->contains(processor);
    }
};

/**
 * Sampler that returns positive every tenth run
 */
INJECTION_SAMPLER_RS(VNVPACKAGENAME, And, sampleRunner, R"(
{
    "type" : "object",
    "properties" : {
        "first" : {"type" : "string", "description" : "Only Run if both are true" }        
    }
})") {
    return runner->run(config, comm, type, stageId);
}
