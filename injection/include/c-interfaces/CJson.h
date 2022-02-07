#ifndef JSONCINTERFACE_H
#define JSONCINTERFACE_H


#include "c-interfaces/PackageName.h"
#include "json-c-interface.hpp"

#if __cplusplus
      #define EXTERNC extern "C"
#else 
      #define EXTERNC
      #define INJECTION_OPTIONS(PNAME, schema)                                                      \
      void* REG_HELPER(_VnV_options_callback_, PNAME)(cjson json);                               \
      void REG_HELPER(_VnV_register_options_, PNAME)() {                                          \
          _VnV_registerOptions(VNV_STR(PNAME), schema, &REG_HELPER(_VnV_options_callback_, PNAME)); \
      }                                                                                           \
      void* REG_HELPER(_VnV_options_callback_, PNAME)(cjson json)

      #define REGISTEROPTIONS(PNAME) REG_HELPER(_VnV_register_options_, PNAME)();
      #define DECLAREOPTIONS(PNAME) void REG_HELPER(_VnV_register_options_, PNAME)();

#endif  


EXTERNC typedef void* (*options_callback_ptr)(cjson info);
EXTERNC typedef char* (*options_schema_ptr)();
EXTERNC void _VnV_registerOptions(const char* packageName, const char* sptr, options_callback_ptr ptr);

#  endif  // JSONINTERFACE_H
