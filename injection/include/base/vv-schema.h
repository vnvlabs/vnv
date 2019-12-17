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
#include "json-schema.hpp"
using nlohmann::json;

namespace VnV {

/**
 * @brief getVVSchema
 * @return
 */
const json& getVVSchema();

/**
 * @brief getTestDelcarationJsonSchema
 * @return
 */
json getTestDelcarationJsonSchema();

/**
 * @brief getTestValidationSchema
 * @param testDeclaration
 * @return
 */
json getTestValidationSchema(json &testDeclaration);

json getTransformDeclarationSchema() ;

json getInjectionPointDeclarationSchema();

//json getTestMappingSchema(json injectionPointDeclaration);

}
