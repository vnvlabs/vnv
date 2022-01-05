
/** @file JsonSchema.cpp Implementation of the JsonSchema class as defined in
    base/JsonSchema.h
**/
#include "base/parser/JsonSchema.h"

#include "base/exceptions.h"
#include "json-schema.hpp"
using nlohmann::json_schema::json_validator;

namespace VnV {

const json& getVVSchema() {
  static const json __vv_schema__ = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "http://rnet-tech.net/vv.schema.json",
  "title": "Basic VV RunTime Schema",
  "description": "Schema for a VnV runtime Configuration file",
  "type": "object",
  "properties": {
    "runTests": {
      "type": "boolean"
    },
    "schema" : {
      "type" : "object",
      "properties" : {
        "dump" : {"type" : "boolean", "default" : false },
        "quit" : {"type" : "boolean", "default" : false }
      },
      "additionalProperties" : false
    },
    "logging": {
      "$ref": "#/definitions/logger"
    },
    "unit-testing": {
      "$ref": "#/definitions/unit-testing"
    },
    "actions": {
      "$ref": "#/definitions/actions"
    },
    "additionalPlugins": {
      "$ref": "#/definitions/additionalPlugins"
    },
    "outputEngine": {
      "$ref": "#/definitions/outputEngine"
    },
    "runScopes": {
      "$ref": "#/definitions/runScopes"
    },
    "hot-patch": {
      "type": "boolean"
    },
    "injectionPoints": {
      "$ref": "#/definitions/injectionPoints"
    },
    "iterators": {
      "$ref": "#/definitions/iterators"
    },
    "plugs": {
      "$ref": "#/definitions/plugs"
    },
    "options": {
      "$ref" : "#/definitions/options"
    },
    "template-overrides": {
      "type": "object"
    },
    "communicator": {
      "$ref": "#/definitions/communicator"
    }
  },
  "additionalProperties": false,
  "definitions": {
    "additionalPlugins": {
      "description": "Map describing all plugins in the system",
      "type": "object",
      "additionalProperties": {
        "type": "string"
      },
      "vnv-file": true,
      "default": {}
    },
    "communicator": {
      "type": "string",
      "default": "mpi"
    },
    "outputEngine": {
      "description": "Parameters to configure the output Engine",
      "type": "object",
      "minProperties": 1,
      "maxProperties": 1,
      "default": {
        "json_stdout": {}
      }
    },
    "runScopes": {
      "description": "List of scopes to run. Empty indicates all scopes should run. Type an invalid runscope to turn of all e.g. none ",
      "type": "array",
      "items": {
        "type": "string"
      }
    },
    "unit-testing": {
      "default": {
        "runTests": false
      },
      "type": "object",
      "properties": {
        "runTests": {
          "type": "boolean",
          "default": true
        },
        "config": {
          "default": {},
          "type": "object",
          "additionalProperties": {
            "type": "object",
            "properties": {
              "runUnitTests": {
                "type": "boolean"
              },
              "unitTests": {
                "type": "object",
                "additionalProperties": {
                  "type": "boolean"
                }
              }
            }
          }
        },
        "exitAfter": {
          "type": "boolean",
          "default": true
        }
      }
    },
    "actions": {
      "default": {},
      "type": "object",
      "description": "Actions to be performed during the simulation"
    },
    "logger": {
      "description": "VnV Logging Configuration",
      "type": "object",
      "properties": {
        "on": {
          "type": "boolean"
        },
        "filename": {
          "type ": "string"
        },
        "logs": {
          "$ref": "#/definitions/logTypes"
        },
        "blackList": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      },
      "required": [
        "on",
        "filename",
        "logs"
      ],
      "default": {
        "on": true,
        "filename": ".vnv-logs",
        "logTypes": {}
      }
    },
    "options" : { "type" : "object" },
    "logTypes": {
      "default": {},
      "type": "object",
      "additionalProperties": {
        "type": "boolean",
        "default": true
      }
    },
    "injectionPoints": {
      "description": "Injection Points",
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "$ref": "#/definitions/injectionPoint"
        },
        "additionalProperties": false
      },
      "default": {}
    },
    "runScope": {
      "type": "array",
      "items": {
        "type": "string"
      }
    },
    "injectionPoint": {
      "description": "An injection Point defined somewhere in the code",
      "type": "object",
      "properties": {
        "template": {
          "$ref": "#/definitions/runtemplate"
        },
        "runInternal": {
          "type": "boolean",
          "default": true
        },
        "sampler": {
          "$ref": "#/definitions/sampler"
        },
        "runScope": {
          "$ref": "#/definitions/runScope"
        },
        "tests": {
          "type": "object",
          "patternProperties": {
            "^.*?:.*?$": {
              "$ref": "#/definitions/test"
            }
          },
          "additionalProperties": false
        }
      },
      "default": {
        "runInternal": true,
        "tests": {}
      }
    },
    "sampler": {
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "type": "object"
        }
      },
      "additionalProperties": false
    },
    "iterators": {
      "description": "Injection Iteration Points",
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "$ref": "#/definitions/iterator"
        },
        "additionalProperties": false
      }
    },
    "iterator": {
      "description": "An injection iterator defined somewhere in the code",
      "type": "object",
      "properties": {
        "runInternal": {
          "type": "boolean"
        },
        "template": {
          "$ref": "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "tests": {
          "type": "object",
          "patternProperties": {
            "^.*?:.*?$": {
              "$ref": "#/definitions/test"
            }
          },
          "additionalProperties": false
        },
        "iterators": {
          "type": "object",
          "patternProperties": {
            "^.*?:.*?$": {
              "$ref": "#/definitions/iteratorfunc"
            }
          },
          "additionalProperties": false
        }
      },
      "default": {
        "runInternal": true,
        "tests": {},
        "iterators": {}
      }
    },
    "plugs": {
      "description": "Plugs",
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "$ref": "#/definitions/plug"
        }
      },
      "additionalProperties": false
    },
    "plug": {
      "description": "An injection plug defined somewhere in the code",
      "type": "object",
      "properties": {
        "template": {
          "$ref": "#/definitions/runtemplate"
        },
        "runInternal": {
          "type": "boolean"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "tests": {
          "type": "object",
          "patternProperties": {
            "^.*?:.*?$": {
              "$ref": "#/definitions/test"
            }
          },
          "additionalProperties": false
        },
        "plug": {
          "$ref": "#/definitions/plugger"
        }
      },
      "default": {
        "runInternal": true,
        "tests": {},
        "plug": {}
      }
    },
    "plugged": {
      "type": "object",
      "properties": {
        "config": {
          "type": "object"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      }
    },
    "plugger": {
      "description": "Test information",
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "$ref": "#/definitions/plugged"
        }
      },
      "maxProperties": 1,
      "minProperties": 1,
      "additionalProperties": false
    },
    "iteratorfunc": {
      "description": "Test information",
      "type": "object",
      "patternProperties": {
        "^.*?:.*?$": {
          "$ref": "#/definitions/iteratorconfig"
        }
      },
      "maxProperties": 1,
      "minProperties": 1,
      "additionalProperties": false
    },
    "iteratorconfig": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "config": {
          "type": "object"
        },
        "template": {
          "$ref": "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      },
      "default": {
        "config": {}
      }
    },
    "test": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "config": {
          "type": "object"
        },
        "template": {
          "$ref": "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      },
      "default": {
        "config": {}
      },
      "additionalProperties": false
    },
    "runtemplate": {
      "type": "array",
      "items": {
        "type": "object",
        "additionalProperties": {
          "type": "string"
        }
      }
    }
  }
}
)"_json;

  return __vv_schema__;
}

json& getBaseOptionsSchema() {
  static json __base_options_schema__ = R"({
         "type" : "object",
         "properties" : {
             "logUnhandled" : { "type" : "boolean" },
             "dumpConfig" : {"type" : "string" },
             "exitAfterDumpConfig" : {"type" : "boolean"}
         },
         "additionalProperties" : false
      }
)"_json;

  return __base_options_schema__;
}

json& getDefaultOptionsSchema() {
  static json __default_options_schema__ = R"({"type" : "object"})"_json;
  return __default_options_schema__;
}

json getTestValidationSchema(json& optsschema) {
  json schema = R"(
    {
       "$schema": "http://json-schema.org/draft-07/schema#",
       "type": "object"
    })"_json;

  schema["title"] = "Test Declaration Schema";
  schema["description"] = "Schema for verifying test. ";

  json properties = R"({})"_json;
  properties["configuration"] = optsschema;

  json parameters = R"({"type":"object" ,"properties" : {}, "additionalProperties" : false})"_json;
  parameters["required"] = json::array();

  properties["parameters"] = parameters;
  schema["properties"] = properties;
  schema["required"] = R"(["configuration"])"_json;
  return schema;
}

bool validateSchema(const json& config, const json& schema, bool throwOnInvalid) {
  json_validator validator;
  validator.set_root_schema(schema);
  try {
    validator.validate(config);
    return true;
  } catch (std::exception e) {
    if (throwOnInvalid) {
      throw INJECTION_EXCEPTION("Schema validation failed: %s", e.what());
    } else {
      return false;
    }
  }
}

}  // namespace VnV
