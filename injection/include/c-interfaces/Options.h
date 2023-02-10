#ifndef JSONCINTERFACE_H
#define JSONCINTERFACE_H

#include "common-interfaces/PackageName.h"
#include "json-c-interface.hpp"

typedef void* (*options_callback_ptr)(cjson info);
typedef char* (*options_schema_ptr)();

VNVEXTERNC void _VnV_registerOptions(const char* packageName, const char* sptr, options_callback_ptr ptr);
VNVEXTERNC void* _VnV_getOptionsObject(const char* package);

#ifndef __cplusplus

#  define INJECTION_OPTIONS(PNAME, schema, OptionsStruct)                                       \
    void* REG_HELPER(_VnV_options_callback_, PNAME)(cjson json);                                \
    void REG_HELPER(_VnV_register_options_, PNAME)() {                                          \
      _VnV_registerOptions(VNV_STR(PNAME), schema, &REG_HELPER(_VnV_options_callback_, PNAME)); \
    }                                                                                           \
    OptionsStruct* REG_HELPER(_VnV_get_options_, PNAME)() {                                     \
      return (OptionsStruct*)_VnV_getOptionsObject(VNV_STR(PNAME));                             \
    }                                                                                           \
    void* REG_HELPER(_VnV_options_callback_, PNAME)(cjson json)

#  define REGISTEROPTIONS(PNAME) REG_HELPER(_VnV_register_options_, PNAME)();
#  define DECLAREOPTIONS(PNAME) void REG_HELPER(_VnV_register_options_, PNAME)();
#  define INJECTION_GET_CONFIG(PNAME) REG_HELPER(_VnV_get_options_, PNAME)();

#endif

#endif  // JSONINTERFACE_H
