
/** @file JsonSchema.cpp Implementation of the JsonSchema class as defined in
    base/JsonSchema.h
**/
#include "base/JsonSchema.h"

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
    "unit-testing" : {
       "$ref" : "#/definitions/unit-testing"
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
    },
    "toolConfig" : {
      "type" : "object"
    }
  },
  "additionalProperties": false,
  "definitions": {
    "testLibraries": {
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
          "config" : {"type" : "object"}
       },
       "required" : ["runTests","config"]
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
        "runInternal": {
           "type" : "boolean"
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


static json __doc_declaration_schema__ = R"(
{
 "$schema": "http://json-schema.org/draft-07/schema#",
 "$id": "http://rnet-tech.net/vv.schema.json",
 "title": "doc declaration schema",
 "type": "array",
 "items" : {
    "type":"object",
    "properties" : {
     "name" : {"type" : "string"},
     "parameters" :{ "type": "object",
                     "additionalProperties" : { "type" : "string" }
                   }
      },
      "required" : ["name"]
  }
}
)"_json ;

json& getDocumentationSchema() {
    return __doc_declaration_schema__;
}

//Get rid of this
static json __base_options_schema__ = R"({
         "type" : "object",
         "parameters" : {
             "logUnhandled" : { "type" : "boolean" }
         },
         "additionalParameters" : false
      }
)"_json;

json& getBaseOptionsSchema() {
    __base_options_schema__.dump(3);
    return __base_options_schema__;
}


static json __transform_declaration_schema__ = R"({
 "$schema": "http://json-schema.org/draft-07/schema#",
 "$id": "http://rnet-tech.net/vv.transform.schema.json",
 "title": "transform declaration schema",
 "type": "object",
 "additionalProperties" : {"type" : "array" , "items" : { "type" : "string" } }
}
)"_json ;



json getTransformDeclarationSchema() {
    return __transform_declaration_schema__;
}

static json __injectionPoint_declaration_schema__ = R"({
"$schema": "http://json-schema.org/draft-07/schema#",
"$id": "http://rnet-tech.net/vv.injectionPoint.schema.json",
"title": "injection point declaration schema",
"type": "object",
"properties" : {
     "name" : {"type":"string"},
     "parameters" : {"$ref" : "#/definitions/parameters"},
     "documentation" : {"$ref" : "#/definitions/documentation"}
},
"required" : ["name","parameters"],
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

json getInjectionPointDeclarationSchema() {
    return __injectionPoint_declaration_schema__;
}


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
       "type": "object"
    })"_json;

    schema["title"] = testDeclaration["title"];
    schema["description"] = testDeclaration["description"];

    json properties = R"({})"_json;
    properties["expectedResult"] = testDeclaration["expectedResult"];
    properties["configuration"] = testDeclaration["configuration"];

    json parameters = R"({"type":"object" ,"properties" : {}, "additionalProperties" : false})"_json;
    parameters["required"] = testDeclaration["requiredParameters"];
    for ( auto it : testDeclaration["parameters"].items()) {
        parameters["properties"][it.key()] = R"({"type":"string"})"_json;
    }

    properties["parameters"] = parameters;
    schema["properties"] = properties;
    schema["required"] = R"(["expectedResult","configuration","parameters"])"_json;
    return schema;
}


}
