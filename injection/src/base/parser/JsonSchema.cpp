
/** @file JsonSchema.cpp Implementation of the JsonSchema class as defined in
    base/JsonSchema.h
**/
#include "base/parser/JsonSchema.h"

#include <iostream>

#include "shared/exceptions.h"
#include "json-schema.hpp"
using nlohmann::json_schema::json_validator;

namespace VnV {

const json& getVVSchema() {
  try {
    static const json __vv_schema__ = json::parse(
#include "base/parser/schema.json"
    );

    return __vv_schema__;

  } catch (json::parse_error& ex) {
    throw INJECTION_EXCEPTION("Error Parsing VnV Schema %s", ex.what());
  }
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
  } catch (std::exception& e) {
    if (throwOnInvalid) {
      throw INJECTION_EXCEPTION("Schema validation failed: %s", e.what());
    } else {
      return false;
    }
  }
}

}  // namespace VnV
