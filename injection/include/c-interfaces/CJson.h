#ifndef JSONINTERFACE_H
#define JSONINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/PackageName.h"

struct _c_json {
  void* data;
};

typedef struct _c_json c_json;
typedef void (*options_callback_ptr)(c_json info);
typedef char* (*options_schema_ptr)();

#  if __cplusplus
#    include "interfaces/ICommunicator.h"
#    include "interfaces/IOutputEngine.h"
#    include "json-schema.hpp"
using nlohmann::json;
namespace VnV {
json* asJson(c_json json);
typedef void (*options_cpp_callback_ptr)(json& info, IOutputEngine* engine, ICommunicator_ptr world);

void RegisterOptions(std::string packageName, std::string schema, options_cpp_callback_ptr callback);
void RegisterOptions_Json(std::string packageName, json& schema, options_cpp_callback_ptr callback);
}  // namespace VnV
#    define EXTERNC extern "C"
#  else
#    define EXTERNC
#    include <stdbool.h>
#  endif

EXTERNC void VnV_CJson_is_null(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_is_object(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_getKeyValue(c_json json, char* key, c_json* result, int* err);
EXTERNC void VnV_CJson_hasKey(c_json json, char* key, bool* result, int* err);
EXTERNC void VnV_CJson_getAllKeys(c_json json, char*** keys, int* size, int* err);
EXTERNC void VnV_CJson_freeAllKeys(c_json json, char*** keys, int* size, int* err);
EXTERNC void VnV_CJson_is_array(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_getArraySize(c_json json, unsigned long* result, int* err);
EXTERNC void VnV_CJson_getArrayIndex(c_json json, unsigned int index, c_json* result, int* err);
EXTERNC void VnV_CJson_is_number(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_is_number_float(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_is_number_unsigned(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_is_number_integer(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_getInt(c_json json, int* result, int* err);
EXTERNC void VnV_CJson_getUnsignedInt(c_json json, unsigned int* result, int* err);
EXTERNC void VnV_CJson_getFloat(c_json json, float* result, int* err);
EXTERNC void VnV_CJson_is_string(c_json json, int* err);
EXTERNC void VnV_CJson_getString(c_json json, char** result, int* err);
EXTERNC void VnV_CJson_freeString(c_json json, char** result, int* err);
EXTERNC void VnV_CJson_is_boolean(c_json json, int* err);
EXTERNC void VnV_CJson_getBoolean(c_json json, bool* result, int* err);
EXTERNC void VnV_CJson_printJson(c_json json, int* err);
EXTERNC void VnV_CJson_getErrorMessage(int err, char** message);

EXTERNC void _VnV_registerOptions(const char* packageName, const char* sptr, options_callback_ptr ptr);

#  define REGISTER_OPTIONS_RAW(PNAME, SCHEMA) __VnV_registerOptions(#  PNAME, SCHEMA, NULL);

#  if __cplusplus

#    define INJECTION_OPTIONS(PNAME, schema)                                                        \
      namespace VnV {                                                                               \
      namespace PNAME {                                                                             \
      void optionsCallback(json& config, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world); \
      void registerOptions() { VnV::RegisterOptions(VNV_STR(PNAME), schema, &optionsCallback); }    \
      }                                                                                             \
      }                                                                                             \
      void VnV::PNAME::optionsCallback(json& config, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world)

#    define DECLAREOPTIONS(PNAME) \
      namespace VnV {             \
      namespace PNAME {           \
      void registerOptions();     \
      }                           \
      }

#    define REGISTEROPTIONS(PNAME) VnV::PNAME::registerOptions();

#  else

#    define INJECTION_OPTIONS(PNAME, schema)                                                      \
      void REG_HELPER(_VnV_options_callback_, PNAME)(c_json json);                                \
      void REG_HELPER(_VnV_register_options_, PNAME)() {                                          \
        _VnV_registerOptions(VNV_STR(PNAME), schema, &REG_HELPER(_VnV_options_callback_, PNAME)); \
      }                                                                                           \
      void REG_HELPER(_VnV_options_callback_, PNAME)(c_json json)

#    define REGISTEROPTIONS(PNAME) REG_HELPER(_VnV_register_options_, PNAME)();
#    define DECLAREOPTIONS(PNAME) void REG_HELPER(_VnV_register_options_, PNAME)();

#  endif

#else
#  define VnV_Register_Options(...)
#endif

#endif  // JSONINTERFACE_H
