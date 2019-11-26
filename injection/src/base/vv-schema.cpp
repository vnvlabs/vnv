
#include "vv-schema.h"
#include <iostream>

namespace VnV {
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
    "testLibraries": {
      "$ref": "#/definitions/testLibraries"
    },
    "outputEngine": {
      "$ref": "#/definitions/outputEngine"
    },
    "runScopes": {
      "$ref": "#/definitions/runScopes"
    },
    "injectionPoints": {
      "$ref": "#/definitions/injectionPoints"
    }
  },
  "additionalProperties": false,
  "definitions": {
    "testLibraries": {
      "description": "Array containing the paths to  the plugin libraries to be imported",
      "type": "array",
      "items": {
        "type": "string"
      },
      "uniqueItems": true
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
    "properties" : {
        "WARN" : {"type" : "boolean"},
        "INFO" : {"type" : "boolean"},
        "DEBUG" : {"type" : "boolean"},
        "ERROR" : {"type" : "boolean"}
    },
    "additionalProperties" : false
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
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        }
      },
      "required": [
        "name","tests"
      ]
    },
    "test": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "config" : {
            "type" : "object"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          },
          "minLength": 1
        }
      },
      "required": [
        "name",
        "runScope"
      ],
      "additionalProperties": false
    }
  }
})"_json;

const json& getVVSchema() {
    return __vv_schema__;
}

static json __test_declaration_schema__ = R"(
{
 "$schema": "http://json-schema.org/draft-07/schema#",
 "$id": "http://rnet-tech.net/vv.schema.json",
 "title": "test declaration schema",
 "type": "object",
 "properties" : {
     "configuration" : {"$ref":"#/raw"},
     "expectedResult" : {"$ref":"#/raw"},
     "description" : {"type" : "string"},
     "title" : {"type" : "string" },
     "supports" : {"oneOf":[{"type" : "array" , "items" : {"type" : "string", "enum" : ["Linux","MacOs"] }}, {"type" : "string", "enum" : ["all"]}]},
     "parameters" :{ "type": "object",
                     "additionalProperties" : { "type" : "string" }
                   }
      },
      "requiredParameters" : {"type":"array", "items" : {"type": "string" },
      "io-variables" : { "type" : "object" , "additionalProperties" : {"type" : "string", "enum" : ["String","Double","Float","Long","Integer"]} }
  },
  "required" : ["expectedResult","configuration","description","title","parameters","requiredParameters","io-variables"],
  "definitions" : {}
}
)"_json ;

json getTestDelcarationJsonSchema() {
    if (__test_declaration_schema__.find("raw") == __test_declaration_schema__.end())
        __test_declaration_schema__["raw"] = nlohmann::json_schema::draft7_schema_builtin;
    return __test_declaration_schema__;
}


json getTestValidationSchema(json &testDeclaration) {
    json schema = R"(
    {
       "$schema": "http://json-schema.org/draft-07/schema#",
       "$id": "http://rnet-tech.net/vv.schema.json",
       "type": "object",
       "definitions": {
          "testStage": {
             "properties" : {
                   "parameter" : {"type" : "string" },
                   "transform" : { "oneOf" : [{"type" : "array", "items" : { "type" : "string" }}, {"type" : "string"} ]}
               },
               "type": "object",
               "required" : ["parameter","transform"]
          }
        }
    })"_json;

    schema["title"] = testDeclaration["title"];
    schema["description"] = testDeclaration["description"];

    json properties = R"({})"_json;
    properties["expectedResult"] = testDeclaration["expectedResult"];
    properties["configuration"] = testDeclaration["configuration"];

    json parameters = R"({"type":"object" ,"properties" : {}, "additionalProperties" : false})"_json;
    parameters["required"] = testDeclaration["requiredParameters"];
    for ( auto it : testDeclaration["parameters"].items()) {
        parameters["properties"][it.key()] = R"({"oneOf" : [{"$ref" : "#/definitions/testStage"},{"type":"string"}]})"_json;
    }

    properties["parameters"] = parameters;
    schema["properties"] = properties;
    schema["required"] = R"(["expectedResult","configuration","parameters"])"_json;
    return schema;
}


}
