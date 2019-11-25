/**
 *\file vv-schema.h
 *
 * Json Schema file for the VV json format. This is stored as a string Raw
 *string literal, and converted to json with the custom _json string literal
 *prefix provided by nlohmann/json.hpp.
 *
 * The file including the schema should load nlohmann/json prior to including
 *it.
 *
 */

static json vv_schema = R"(
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
        "pattern": "^(.*).(so|dylib)$"
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
