
/**
  @file Injection.cpp
**/
#include <stdarg.h>
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/ITest.h"

#include "c-interfaces/CppInjection.h"

using namespace VnV;

// Private function -- Never exposed to the C interface.
NTV VnV_UnwrapVariadicArgs(va_list argp) {
    NTV parameterSet;
    while (1) {
      std::string variableName = va_arg(argp, char*);
      if (variableName == VNV_END_PARAMETERS_S) {
        break;
      }
      void* variablePtr = va_arg(argp, void*);

      //variable was not registered, add it with a type void*
      parameterSet.insert(std::make_pair(variableName, std::make_pair("void*", variablePtr)));

    }
    return parameterSet;
}

extern "C" {

void _VnV_injectionPoint(const char *package, const char* id, ...) {
  va_list argp;
  va_start(argp, id);
  NTV map = VnV_UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint(package, id, map);
  va_end(argp);
}

void _VnV_injectionPoint_begin(const char *package, const char* id, ...) {
  va_list argp;
  va_start(argp, id);
  NTV map = VnV_UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint_begin(package, id, map);
  va_end(argp);
}

void _VnV_injectionPoint_end(const char * package, const char* id){
  VnV::RunTime::instance().injectionPoint_end(package, id);
}

void _VnV_injectionPoint_loop(const char * package, const char* id, const char* stageId){
  VnV::RunTime::instance().injectionPoint_iter(package, id, stageId);
}

void _VnV_registerInjectionPoint( const char *json_str) {
   VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(json_str);
}


}
