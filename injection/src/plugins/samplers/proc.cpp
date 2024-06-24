
/** Include the VnV include file **/
#include "VnV.h"



class sampleRunner
{
private:
    bool setup_complete = false;
    int processor = 0;
    void setup(json &config)
    {
        if (!setup_complete)
        {
            processor = config.template value("processor", 0);
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
INJECTION_SAMPLER_RS(VNVPACKAGENAME, Processor, sampleRunner, R"(
{
    "type" : "object",
    "properties" : {
        "proc" : {"type" : "number", "min" : 0, "default" : 0 , "description" : "Only Run if it involves this processor" }        
    }
})") {
    return runner->run(config, comm, type, stageId);
}
