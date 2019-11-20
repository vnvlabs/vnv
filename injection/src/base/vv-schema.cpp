
#include "vv-schema.h"

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
        "type": "string",
        "pattern": "^(.*).so$"
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
        "logs" : { "$ref" : "#/definitions/logTypes" }
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
      "type": "object",
      "properties" : {
          "config" : {
             "type" : "array",
             "items": {
              "$ref": "#/definitions/injectionPoint"
            }
          },
          "files" : {
            "description" : "Injection point config files",
            "type" : "array",
            "items" : { "type" : "string" }
          }
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
        "name"
      ]
    },
    "test": {
      "description": "Test information",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "stages": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/definitions/testStage"
          },
          "propertyNames": {
            "pattern": "^[0-9]*$"
          }
        },
        "testConfig": {
          "type": "object"
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
        "stages",
        "runScope"
      ],
      "additionalProperties": false
    },
    "testStage": {
      "type": "object",
      "properties": {
        "ipId": {
          "type": "integer"
        },
        "mapping": {
          "type": "object",
          "additionalProperties": {
            "type": "object",
            "properties": {
              "ipParameter": {
                "type": "string"
              },
              "transform": {
                "type": "string"
              }
            },
            "required": [
              "ipParameter"
            ]
          }
        },
    "expected": { "type" : "object" }
      },
      "required": [
        "ipId",
        "mapping"
      ]
    }
  }
})"_json;

const json& getVVSchema() {
    return __vv_schema__;
}
static json __test_declaration_schema__ = R"({
   "$schema": "http://json-schema.org/draft-07/schema#",
   "$id": "http://rnet-tech.net/vv.schema.json",
   "title": "test declaration schema",
   "type": "object",
   "properties" : {
       "configuration" : {"$ref":"#/raw"},
       "expectedResult" : {"$ref":"#/raw"},
       "description" : {"type" : "string"},
       "title" : {"type" : "string" },
       "stages" :{ "type": "object",
                   "additionalProperties" : {
                       "type" : "object",
                       "additionalProperties" : {"type":"string"}
                   },
                   "propertyNames" : {
                     "pattern":"^-?[0-9]*$"
                   }
                 }
        },
        "requiredStages" : {"type":"array", "items" : {"type": "string" },
        "io-variables" : { "type" : "object" , "additionalProperties" : {"type" : "string"} }
    },
    "required" : ["expectedResult","configuration","description","title","stages","requiredStages","io-variables"],
    "definitions" : {}
})"_json ;

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
         "testStage" : { "type" : "object" ,
                         "additionalItems : { "oneOf" : [{ "type" : "array", "items" : {"type":"string"} },{"type":"string"}] }
         }
       }
    })"_json;

    schema["title"] = testDeclaration["title"];
    schema["description"] = testDeclaration["description"];

    json properties = R"({"name":{"type":"string"}})"_json;
    properties["expectedResult"] = testDeclaration["expectedResult"];
    properties["config"] = testDeclaration["configuration"];

    json stages = R"({"type":"object" ,"properties" : {}, "additionalProperties" : false)"_json;

    for ( auto it : testDeclaration["stages"].items()) {
        json stage = R"({ "type"  : "object", "properties" : {"ipId" : { "type" : "integer" } } })"_json;
        for (auto itt : it.value().items()) {
            stage["properties"][itt.key()] = R"("type" : { "oneOf" : [{"$ref" : "#/definitions/testStage",{"type":"string"}]})"_json;
        }
        stage["required"] = it.value()["required"];
        stage["required"].insert(stage["required"].end(), "ipId");
        stages["properties"][it.key()] = stage;
    }
    stages["required"] = testDeclaration["required"];
    properties["stages"] = stages;
    schema["properties"] = properties;
    return schema;
}


}
