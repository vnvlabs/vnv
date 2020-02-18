#ifndef JSONINTERFACE_H
#define JSONINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/packagename.h"

struct _c_json {
    void* data;
};
typedef struct _c_json c_json;

#if __cplusplus
   #include "json-schema.hpp"
   using nlohmann::json;
   namespace VnV {
        json* asJson(c_json json);
   };
   #define EXTERNC extern "C"
#else
   #define EXTERNC
   #include <stdbool.h>
#endif


EXTERNC bool VnV_check(c_json json);
EXTERNC bool VnV_is_null(c_json json);
EXTERNC bool VnV_is_object(c_json json);
EXTERNC bool VnV_getKeyValue(c_json json, char* key, c_json* result);
EXTERNC bool VnV_hasKey(c_json json, char* key);
EXTERNC bool VnV_getAllKeys(c_json json, char***keys, int* size);
EXTERNC bool VnV_freeAllKeys(c_json json, char***keys, int* size);
EXTERNC bool VnV_is_array(c_json json);
EXTERNC bool VnV_getArraySize(c_json json, unsigned int *result);
EXTERNC bool VnV_getArrayIndex(c_json json, unsigned int index, c_json *result);
EXTERNC bool VnV_is_number(c_json json);
EXTERNC bool VnV_is_number_float(c_json json);
EXTERNC bool VnV_is_number_unsigned(c_json json);
EXTERNC bool VnV_is_number_integer(c_json json);
EXTERNC bool VnV_getInt(c_json json, int *result);
EXTERNC bool VnV_getUnsignedInt(c_json json, unsigned int *result);
EXTERNC bool VnV_getFloat(c_json json, float *result);
EXTERNC bool VnV_is_string(c_json json);
EXTERNC bool VnV_getString(c_json json, char **result);
EXTERNC bool VnV_freeString(c_json json, char **result);
EXTERNC bool VnV_is_boolean(c_json json);
EXTERNC bool VnV_getBoolean(c_json json, bool *result);
EXTERNC bool VnV_printJson(c_json json);

typedef void options_callback_ptr(c_json info);
typedef char* options_schema_ptr();


EXTERNC void _VnV_registerOptions(const char *packageName, options_schema_ptr *sptr, options_callback_ptr *ptr);

#define VnV_Register_Options(schema, callback) \
    _VnV_registerOptions(PACKAGENAME_S, schema, callback)


#undef EXTERNC

#else
#define VnV_Register_Options(...)
EXTERNC bool VnV_check(c_json json){return false};
EXTERNC bool VnV_is_null(c_json json){return false};
EXTERNC bool VnV_is_object(c_json json){return false};
EXTERNC bool VnV_getKeyValue(c_json json, char* key, c_json* result){return false};
EXTERNC bool VnV_hasKey(c_json json, char* key){return false};
EXTERNC bool VnV_getAllKeys(c_json json, char***keys, int* size){return false};
EXTERNC bool VnV_freeAllKeys(c_json json, char***keys, int* size){return false};
EXTERNC bool VnV_is_array(c_json json){return false};
EXTERNC bool VnV_getArraySize(c_json json, unsigned int *result){return false};
EXTERNC bool VnV_getArrayIndex(c_json json, unsigned int index, c_json *result){return false};
EXTERNC bool VnV_is_number(c_json json){return false};
EXTERNC bool VnV_is_number_float(c_json json){return false};
EXTERNC bool VnV_is_number_unsigned(c_json json){return false};
EXTERNC bool VnV_is_number_integer(c_json json){return false};
EXTERNC bool VnV_getInt(c_json json, int *result){return false};
EXTERNC bool VnV_getUnsignedInt(c_json json, unsigned int *result){return false};
EXTERNC bool VnV_getFloat(c_json json, float *result){return false};
EXTERNC bool VnV_is_string(c_json json){return false};
EXTERNC bool VnV_getString(c_json json, char **result){return false};
EXTERNC bool VnV_freeString(c_json json, char **result){return false};
EXTERNC bool VnV_is_boolean(c_json json){return false};
EXTERNC bool VnV_getBoolean(c_json json, bool *result){return false};
#endif


#endif // JSONINTERFACE_H
