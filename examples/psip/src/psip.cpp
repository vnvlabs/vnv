
#include "VnV.h"

class PSIPAction : public VnV::IAction {
  std::string uri;

 public:
  PSIPAction(json config) { uri = config["uri"].get<std::string>(); }

  virtual void initialize() override { getEngine()->Put("uri", uri); }
};

const char* schema = R"(
    {
        "type" : "object",
        "properties" : {
            "uri" : {
                "type" : "string",
                "description": "URI To A PSIP Tracking file"
            }
        }, 
        "required" : ["uri"]       
    }
)";

/**
 * .. vnv-psip:: uri[0]
 */
INJECTION_ACTION(PSIP, psip, schema) { return new PSIPAction(config); }
