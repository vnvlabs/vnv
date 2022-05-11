
#include "VnV.h"

class PSIPAction : public VnV::IAction {
  json conf;

 public:
  PSIPAction(const json& config) { 
    conf = config;
  }

  json getDefault() {
    return R"(
{
   "sa" : [
      [0,0,0,0,0],
      [0,0,0,0,0,0],
      [0,0,0],
      [0,0],
      [0,0]
   ],
   "ptc" : [
    {
      "process" : "Start PSIP Journey",
      "target" : "This PSIP Card focusing on integrating PSIP into your workflow.",
      "score" : 0,
      "scores" : [ "No PSIP Integration", "Some PSIP Integration" , "A lot of PSIP Integration", "Woah -- All the PSIP integration" ]
    }
   ]
}
)"_json;
  }

  virtual void initialize() override {
    if (conf.size() == 0) {
      conf = getDefault();
    }
    getEngine()->Put("psip", conf);
  }
};

//TODO WRITE A SCHEMA
const char* schema = R"(
{
        "type" : "object",
        "properties" : {}
}
)";

/**
 * .. vnv-psip:: psip
 */
INJECTION_ACTION(VNVPACKAGENAME, PSIP, schema) { 
  return new PSIPAction(config);
}
