﻿
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
    "runTests" : {
        "type" : "boolean"
    },
    "logging" : {
       "$ref": "#/definitions/logger"
    },
    "unit-testing" : {
       "$ref" : "#/definitions/unit-testing"
    },
    "actions" : {
       "$ref" : "#/definitions/actions"
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
    "hot-patch" : {
      "type" : "boolean"
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
    "options" : {
      "type" : "object"
    },
    "template-overrides" : {
      "type" : "object"
    },
    "communicator" : {"$ref" : "#/definitions/communicator" }
  },
  "additionalProperties": false,
  "definitions": {
    "additionalPlugins": {
      "description": "Map describing all plugins in the system",
      "type": "object",
      "additionalProperties" : {"type" : "string" }
    },
    "communicator" : {"type" : "string" , "default" : "mpi" },
    "outputEngine": {
      "description": "Parameters to configure the output Engine",
      "type": "object",
      "properties": {
        "type": {
          "type": "string",
          "description": "Registered Typename for the engine"
        },
        "config": {
          "type": "object",
          "description": "Additional Configuration Properties to be sent to the Engine"
        }
      },
      "required": [
        "type"
      ],
      "additionalProperties": false
    },
    "runScopes": {
      "description": "List of scopes to run. Empty indicates all scopes should run. Type an invalid runscope to turn of all e.g. none ",
      "type": "array",
      "items" : {"type" : "string" } 
    },
    "unit-testing" : {
       "type" : "object",
       "properties" : {
          "runTests" : {"type" : "boolean"},
          "config" : {
                "type" : "object",
                "additionalProperties" : {
                     "type" : "object",
                     "properties" : {
                         "runUnitTests" : {"type" : "boolean"},
                         "unitTests" : {
                            "type" : "object",
                            "additionalProperties" : { "type" : "boolean" }
                         }
                     }
                }
          },
          "exitAfter" : {"type" : "boolean" }
       }
    },
    "actions" : {
      "type" : "object"
    },
    "logger": {
        "description" : "VnV Logging Configuration",
        "type" : "object",
        "properties" : {
            "on" : { "type" : "boolean" } ,
            "filename" : { "type " : "string" },
            "logs" : { "$ref" : "#/definitions/logTypes" },
            "blackList" : { "type" : "array", "items" : { "type" : "string" } }
        },
        "required" : ["on","filename","logs"]
    },
    "logTypes": {
        "type" : "object",
        "additionalProperties" : {"type" : "boolean" }
    },
    "injectionPoints": {
      "description": "Injection Points",
      "type": "array",
      "items" : {
         "$ref": "#/definitions/injectionPoint"
       }
    },
    "runScope" : {
      "type": "array",
          "items": {
            "type": "string"
          }
    },
    "injectionPoint": {
      "description": "An injection Point defined somewhere in the code",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
       "template" : {
          "$ref" : "#/definitions/runtemplate"
        },
        "runInternal": {
           "type" : "boolean"
        },
        "sampler" : {
           "$ref" : "#/definitions/sampler"
        },
        "runScope": {
          "$ref" : "#/definitions/runScope"
        },
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        }
      },
      "required": [
        "name","package"
      ]
    },
    "sampler" : {
      "type" : "object",
      "properties" : {
         "name" : {"type" : "string" },
         "package" : {"type" : "string" },
         "config" : {"type" : "object" }
      },
      "required" : ["name","package"]
    },
    "iterators": {
      "description": "Injection Iteration Points",
      "type": "array",
      "items" : {
         "$ref": "#/definitions/iteratorfunc"
       }
    },
    "iterator": {
      "description": "An injection iterator defined somewhere in the code",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "runInternal": {
           "type" : "boolean"
        },
        "template" : {
          "$ref" : "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        },
        "iterators": {
            "type": "array",
            "items": {
               "$ref": "#/definitions/test"
            }
        } 
      },
      "required": [
        "name","package"
      ]
    },
    "plugs": {
      "description": "Plugs",
      "type": "array",
      "items" : {
         "$ref": "#/definitions/plug"
       }
    },
    "plug": {
      "description": "An injection plug defined somewhere in the code",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "template" : {
          "$ref" : "#/definitions/runtemplate"
        },
        "runInternal": {
           "type" : "boolean"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        },
        "plug" : {
            "$ref": "#/definitions/plugger"
        } 
      },
      "required": [
        "name","package"
      ]
    },
    "plugger": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "config" : {
            "type" : "object"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      }
    },
    "iteratorfunc": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "config" : {
            "type" : "object"
        },
       "template" : {
          "$ref" : "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      }
    },
    "test": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "config" : {
            "type" : "object"
        },
        "template" : {
          "$ref" : "#/definitions/runtemplate"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        }
      },
      "required": [
        "name",
        "package"
      ],
      "additionalProperties": false
    },
    "runtemplate" : {
      "type" : "array",
      "items" : { "type" : "object" , 
                  "additionalProperties" : {
                     "type" : "string"
                  }
      }
    }
  }
})"_json;

  return __vv_schema__;
}

json& getBaseOptionsSchema() {
  static json __base_options_schema__ = R"({
         "type" : "object",
         "parameters" : {
             "logUnhandled" : { "type" : "boolean" },
             "dumpConfig" : {"type" : "string" },
             "exitAfterDumpConfig" : {"type" : "boolean"}
         },
         "additionalParameters" : false
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

  json parameters =
      R"({"type":"object" ,"properties" : {}, "additionalProperties" : false})"_json;
  parameters["required"] = json::array();
  
  
  properties["parameters"] = parameters;
  schema["properties"] = properties;
  schema["required"] = R"(["configuration"])"_json;
  return schema;
}

bool validateSchema(const json& config, const json& schema,
                    bool throwOnInvalid) {
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
