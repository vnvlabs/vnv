
#include "VnV.h"

class PSIPAction : public VnV::IAction {
  json conf;

 public:
  PSIPAction(const json& config) { conf = config; }

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

const char* schema = R"(
{
        "type" : "object",
        "properties" : {
            "sa" : {
                "type" : "array",
                "items" : {"type" : "array" , "items":{"type" : "integer", "min":0, "max":3 } },
                "length" : 5
            },    
        }, 
        "dependencies" : {
            "sa" : {
                "properties": {
                    "ptc" : {
                       "type" : "array",
                       "items" : {
                          "type" : "object",
                            "properties" : {
                                "process" : {"type" : "string"},
                                "target" : {"type" : "string" },
                                "score" : {"type" : "integer", "min" : 0 },
                                "scores" : {"type" : "array" , "items" : {"type" :"string"} }
                            },
                            "required" : ["process","target","score","scores"]
                        },
                        "minLength" : 1
                    }
                },
                "required": ["ptc"]
            }
        }
}
)";

/**
 * .. vnv-psip:: psip[0]
 */
INJECTION_ACTION(PSIP, psip, schema) { return new PSIPAction(config); }
