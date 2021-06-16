
/** @file JsonSchema.cpp Implementation of the JsonSchema class as defined in
    base/JsonSchema.h
**/
#include "base/parser/JsonSchema.h"
#include "json-schema.hpp"
#include "base/exceptions.h"
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
    "offloading" : {
       "$ref": "#/definitions/offload"
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
    }
  },
  "additionalProperties": false,
  "definitions": {
    "additionalPlugins": {
      "description": "Map describing all plugins in the system",
      "type": "object",
      "additionalProperties" : {"type" : "string" }
    },
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
      "description": "",
      "type": "array",
      "items": {
        "type": "object",
        "description": " A single run Scope",
        "properties": {
          "name": {
            "type": "string",
            "minLength": 1
          },
          "run": {
            "type": "boolean",
            "default": true
          }
        },
        "required": [
          "name",
          "run"
        ]
      }
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
      "type" : "object",
      "properties" : {
         "run" : {"type" : "boolean" },
         "config" : { "$ref" : "#/definitions/actionConfig" }
      }
    },
    "actionConfig" : {
       "type" : "array",
       "items" : {
          "type" : "object",
          "properties" : {
              "package" : { "type" : "string" },
              "action" : {"type" : "string" },
              "config" : {"type" : "object" },
              "run" : {"type" : "boolean" }
          }
       }
    },
    "offload": {
       "type":"object",
       "properties" : {
           "on" : {"type":"boolean"},
           "type" : {"type" : "string"},
           "config" : {"type":"object"}
        }
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
        }
      },
      "required": [
        "name","package"
      ]
    },
       "iterators": {
      "description": "Injection Iteration Points",
      "type": "array",
      "items" : {
         "$ref": "#/definitions/iterator"
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
            "$ref": "#/definitions/test"
        } 
      },
      "required": [
        "name","package"
      ]
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
        "offload" : {
          "type" : "boolean"
        },
        "config" : {
            "type" : "object"
        },
        "parameters" : {
            "type" : "object",
            "additionalParameters" : {"type" : "string" }
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
        "package",
        "parameters"
      ],
      "additionalProperties": false
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


json getTransformDeclarationSchema() {
  static json __transform_declaration_schema__ = R"({
 "$schema": "http://json-schema.org/draft-07/schema#",
 "$id": "http://rnet-tech.net/vv.transform.schema.json",
 "title": "transform declaration schema",
 "type": "object",
 "additionalProperties" : {"type" : "array" , "items" : { "type" : "string" } }
}
)"_json;

  return __transform_declaration_schema__;
}

json getInjectionPointDeclarationSchema() {
  static json __injectionPoint_declaration_schema__ = R"({
"$schema": "http://json-schema.org/draft-07/schema#",
"$id": "http://rnet-tech.net/vv.injectionPoint.schema.json",
"title": "injection point declaration schema",
"type": "object",
"properties" : {
     "name" : {"type":"string"},
     "package" : {"type":"string"},
     "iterative" : {"type" : "boolean"},
     "parameters" : {"$ref" : "#/definitions/parameters"},
     "documentation" : {"$ref" : "#/definitions/documentation"}
},
"required" : ["name","package","parameters"],
"additionalProperties" : false,
"definitions" : {
    "parameters" : {
       "type" : "object",
       "additionalProperties" : { "type" : "string" }
    },
    "parameter" : {
      "type" : "object",
      "properties" : {
         "class" : {"type":"string"},
         "documentation" : {"$ref" : "#/definitions/parameterDocumentation"}
     },
       "required" : ["class"]
    },
    "documentation" : {
      "type" : "object",
      "properties" : {
          "package" : {"type" : "string"},
          "type" : {"type":"string","enum" : ["SINGLE","LOOP"]},
          "function" : {"type" : "string"},
          "line" : {"type" : "string"},
          "file" : {"type" : "string"},
          "stages" : { "type" : "object" },
          "parameters" : {
             "type" : "object",
             "additionalProperties" : {"$ref" : "#/definitions/parameterDocumentation"}
          }
       }
     },
    "parameterDocumentation" : {
      "type" : "object",
      "properties" : {
          "package" : {"type" : "string"},
          "line" : {"type" : "string"},
          "file" : {"type" : "string"}
      }
    }
}
})"_json;

  return __injectionPoint_declaration_schema__;
}

json getTestDelcarationJsonSchema() {
  static json __test_declaration_schema__ = R"(
{
 "$schema": "http://json-schema.org/draft-07/schema#",
 "$id": "http://rnet-tech.net/vv.schema.json",
 "title": "test declaration schema",
 "type": "object",
 "properties" : {
     "configuration" : {"$ref":"#/raw"},
     "description" : {"type" : "string"},
     "title" : {"type" : "string" },
     "iterator" : {"type" : "boolean" },
     "supports" : {"oneOf":[{"type" : "array" , "items" : {"type" : "string", "enum" : ["Linux","MacOs"] }}, {"type" : "string", "enum" : ["all"]}]},
     "parameters" :{
           "type": "object",
           "additionalProperties" : { "type" : "string" }
      },
      "requiredParameters" : {
          "type":"array",
          "items" : {"type": "string"}
      }
  },
  "required" : ["configuration","description","title","parameters","iterator","requiredParameters"],
  "definitions" : {}
}
)"_json;

  if (__test_declaration_schema__.find("raw") ==
      __test_declaration_schema__.end())
    __test_declaration_schema__["raw"] =
        nlohmann::json_schema::draft7_schema_builtin;
  return __test_declaration_schema__;
}

json getTestValidationSchema(std::map<std::string, std::string>& params,
                             json& optsschema) {
  json schema = R"(
    {
       "$schema": "http://json-schema.org/draft-07/schema#",
       "$id": "http://rnet-tech.net/vv.schema.json",
       "type": "object"
    })"_json;

  schema["title"] = "Test Declaration Schema";
  schema["description"] = "Schema for verifying test. ";

  json properties = R"({})"_json;
  properties["configuration"] = optsschema;

  json parameters =
      R"({"type":"object" ,"properties" : {}, "additionalProperties" : false})"_json;
  parameters["required"] = json::array();
  for (auto it : params) {
    parameters["properties"][it.first] = R"({"type":"string"})"_json;
    parameters["required"].push_back(it.first);
  }

  properties["parameters"] = parameters;
  schema["properties"] = properties;
  if (params.size() > 0) {
    schema["required"] = R"(["configuration","parameters"])"_json;
  } else {
    schema["required"] = R"(["configuration"])"_json;
  }
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
          throw VnVExceptionBase(e.what());
      } else {
          return false;
      }
  }
}


}  // namespace VnV
