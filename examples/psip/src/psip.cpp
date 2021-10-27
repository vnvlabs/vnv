
#include "VnV.h"

class PSIPAction : public VnV::IAction {
  std::string code;

 public:
  PSIPAction(json config) : { uri = config["uri"].get<std::string>(); }

  virtual void initialize() override { engine->Put('uri', uri) }
};

const char* schema = R"(
    {
        "type" : "object",
        "properties" : {
            "uri" : {
                "type" : "string",
                "description": "URI To A PSIP Tracking file"
            }
        }       
    }
)";

/**
 * .. vnv-psip:: uri
 */
INJECTION_ACTION(VNVPACKAGENAME, stdout, schema) { return new PSIPAction(config); }
