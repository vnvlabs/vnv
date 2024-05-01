
#ifndef CJSON_JSON_INTERFACE_H
#define CJSON_JSON_INTERFACE_H


struct _cjson {
  void* data;
};
typedef struct _cjson cjson;

#if __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC 
#endif



EXTERNC int cjson_ok(int err);

#define CJSON_IS_TYPES X(null) X(object) X(array) X(string) X(number) X(boolean) X(number_float) X(number_integer)
#define X(CJT) EXTERNC int cjson_is_##CJT(cjson* json, int *result);
CJSON_IS_TYPES
#undef X

EXTERNC int cjson_object_contains(cjson* json, const char* key, int* result);
EXTERNC int cjson_object_key(cjson* json, int index, char* result, int max_size);
EXTERNC int cjson_object_size(cjson* json, int* size);
EXTERNC int cjson_object_get(cjson*, const char* key, cjson* result);
EXTERNC int cjson_array_size(cjson* json, int* size);
EXTERNC int cjson_array_get(cjson*, long index, cjson* result);
EXTERNC int cjson_double(cjson* json, double* value);
EXTERNC int cjson_boolean(cjson* json, int* value);
EXTERNC int cjson_long(cjson* json, long* value);
EXTERNC int cjson_integer(cjson* json, int* value);
EXTERNC int cjson_float(cjson* json, float* value);
EXTERNC int cjson_string(cjson* json, char* value, int max_size);
EXTERNC int cjson_dump(cjson* json, int sep);
EXTERNC const char* cjson_error_message(int err);


#endif