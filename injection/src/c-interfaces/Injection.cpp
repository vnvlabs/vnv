
/**
  @file Injection.cpp
**/
#include <stdarg.h>
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/Injection.h"

extern "C" {

void _VnV_injectionPoint(const char *package, const char* id, const char* function, const char* file, int line, ...) {
  va_list argp;
  va_start(argp, line);
  VnV::RunTime::instance().injectionPoint(package, id, function, file, line, argp);
  va_end(argp);
}
void _VnV_injectionPoint_begin(const char * package, const char* id, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  VnV::RunTime::instance().injectionPoint_begin(package, id, function, file, line, argp);
  va_end(argp);

}
void _VnV_injectionPoint_end(const char * package, const char* id, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  VnV::RunTime::instance().injectionPoint_end(package, id, function, file, line, argp);
  va_end(argp);

}
void _VnV_injectionPoint_loop(const char * package, const char* id, const char* stageId, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  VnV::RunTime::instance().injectionPoint_iter(package, id, stageId, function, file, line, argp);
  va_end(argp);

}
void _VnV_registerInjectionPoint(const char* name, const char *json_str) {
   VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(name,json_str);
}

}

