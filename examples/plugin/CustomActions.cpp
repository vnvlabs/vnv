
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

class SampleAction : public VnV::IAction {
nlohmann::json config;


public:
     SampleAction(const nlohmann::json& config)  {
        this->config = config;
     }    
  
     virtual void initialize() {
        getEngine()->Put("config",config);
     }

};

// Actions are parameterless functions that can be executed at runtime by the
// user through the input file. There are designed to support little stand along
// tasks like writing help messages.
INJECTION_ACTION(VnVPlugin, sampleAction, R"({"type":"object"})") {
  return new SampleAction(config);
}
