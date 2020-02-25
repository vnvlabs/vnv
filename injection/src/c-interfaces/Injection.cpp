
/**
  @file Injection.cpp
**/
#include <stdarg.h>
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/Injection.h"

extern "C" {

void _VnV_injectionPoint(const char *package, const char* id, ...) {
  va_list argp;
  va_start(argp, id);
  VnV::RunTime::instance().injectionPoint(package, id, argp);
  va_end(argp);
}
void _VnV_injectionPoint_begin(const char * package, const char* id, ...){
  va_list argp;
  va_start(argp, id);
  VnV::RunTime::instance().injectionPoint_begin(package, id, argp);
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

