
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

const char* oschema = R"({
   "type" : "object",
   "properties" : {
      "value" : { "type" : "number" }
   },
   "required" : ["value"]
})";

/**
 * Packages can define an options schema and callback that allows users to
 * configure the package directly through the input file.
 *
 * See JsonSchema.org for details about json schema.
 *
 */
INJECTION_OPTIONS(VnVPlugin, oschema) {
  double value = config["value"].get<double>();
  // ... configure the package ....
  return NULL;
}


/**
 * @title Custom Plugin Library
 * 
 * This is a custom plugin library example. 
 * 
 */
INJECTION_PLUGIN(VnVPlugin)