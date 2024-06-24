
/** Include the VnV include file **/
#include "VnV.h"



class sampleRunner
{
private:
    bool setup_complete = false;
    int skip = 50;
    int curr=0;
    void setup(json &config)
    {
        if (!setup_complete)
        {
            skip = config.template value("skip", 50);
            setup_complete = true;
        }
    }

public:
    sampleRunner() {}

    bool run(json &config, VnV::ICommunicator_ptr ptr, VnV::InjectionPointType type, std::string stageId)
    {
        setup(config);
        return (curr++ % skip  == 0) ;
    }
};

/**
 * Sampler that returns positive every tenth run
 */
INJECTION_SAMPLER_RS(VNVPACKAGENAME, Skip, sampleRunner, R"({
    "type" : "object",
    "properties" : {
        "skip" : {"type" : "number", "min" : 1, "default" : 10 , "description" : " Run every <skip> times"}        
    }
})")
{
    return runner->run(config, comm, type, stageId);
}
