/**
 * @file JsonSchema.h
 *
 * Json Schema file for the VV json format. This is stored as a string Raw
 * string literal, and converted to json with the _json string literal
 * prefix provided by nlohmann/json.hpp.
 *
 * The file including the schema should load nlohmann/json prior to
 * including it.
 *
 */

#include "validate/json-schema.hpp"  // <nlohmann json library and json validation
using nlohmann::json;

namespace VnV {

/**
 * @brief Get the raw VnV Json Schema.
 *
 * This is the json schema used to validate the user provided input file
 * prior to parsing it. This schema is a general purpose schema with no
 * plugin/test/etc specific validtaions. For a full application schema
 * customized to the current package and plugins, see
 * VnV::RunTime::getFullJsonSchema()
 * @return the low level json schema for the vnv input file.
 */
const json& getVVSchema();

/**
 * @brief Get a json schema for a VnV test.
 *
 * This function builds a json schema for a VnV test. The schema uses the
 * test parameters and schema to build up a single schema that can be used
 * to validate each test configuration block provided by the user in the
 * input file.
 *
 * @param [in] params A Map of parameter name to parameter type.
 * @param [in] schema The schema for the tests configuration options.
 * @return A custom json schema for the test.
 */
json getTestValidationSchema(json& schema);

/**
 * @brief get The schema for the VnV Package configuration options. This
 * schema will be used to validate options provided by the user in
 * the options/VNV block of the input file.
 * @return A Valid json schema for the vnv configuration options.
 */
json& getBaseOptionsSchema();

/**
 * @brief get the default json schema.
 *
 * The default schema is
 * {"type":"object"} which allows for a json object with
 * any parameters.
 * @return the default json schema
 */
json& getDefaultOptionsSchema();

/**
 * @brief Validate a json object against a json schema.
 *
 * @param [in] config The Json to be validated
 * @param [in] schema The Json Schema to validate against
 * @param [in] throwIfInvalid If true an exception is thrown when
 * the parameter is invalid.
 *
 * @return the validaion result, true if valid, false if invalid
 *
 * @throws VnV::VnVBaseException if throwIfInvalid is true and it
 * is is invalid.
 */
bool validateSchema(const json& config, const json& schema, bool throwIfInvalid);

}  // namespace VnV
