
/** Include the VnV include file **/
#include "VnV.h"



class sampleRunner
{
private:
    bool setup_complete = false;
    double percent = 50;
    void setup(json &config)
    {
        if (!setup_complete)
        {
            percent = config.template value<double>("percentage", 50);
            setup_complete = true;
        }
    }

public:
    sampleRunner() {}

    bool run(json &config, VnV::ICommunicator_ptr ptr, VnV::InjectionPointType type, std::string stageId)
    {
        setup(config);
        return 100*rand()/RAND_MAX <= percent;
    }
};

/**
 * Sampler that returns positive a certain percentage of the time. 
 */
INJECTION_SAMPLER_RS(VNVPACKAGENAME, random, sampleRunner, R"({
    "type" : "object",
    "properties" : {
        "percentage" : {"type" : "number", "min" : 0, "max" : 100, "default" : 50 }
    }
})")
{
    return runner->run(config, comm, type, stageId);
}
