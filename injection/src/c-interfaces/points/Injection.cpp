
/**
  @file Injection.cpp
**/
#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/InjectionPointStore.h"

using namespace VnV;

extern "C" {

void _VnV_injectionPoint(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty, 
                         const char* fname, int line,
                         injectionDataCallback* callback, ...) {
  va_list argp;
  va_start(argp, callback);
  NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint(comm, package, id, pretty, fname, line,
                                          callback, map);
  va_end(argp);
}

void _VnV_injectionPoint_begin(VnV_Comm comm, const char* package,
                               const char* fname, int line, const char* id, struct VnV_Function_Sig pretty, 
                               injectionDataCallback* callback, ...) {
  va_list argp;
  va_start(argp, callback);
  NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint_begin(comm, package, id, pretty, fname, line,
                                                callback, map);
  va_end(argp);
}

void _VnV_injectionPoint_loop(const char* package, const char* id,
                              const char* stageId, const char* fname,
                              int line) {
  VnV::RunTime::instance().injectionPoint_iter(package, id, stageId, fname,
                                               line);
}

int _VnV_injectionPoint_end(const char* package, const char* id,
                            const char* fname, int line) {
  VnV::RunTime::instance().injectionPoint_end(package, id, fname, line);
  return true;
}

void _VnV_registerInjectionPoint(const char* package, const char* id,
                                 const char* parameters) {
  VnV::InjectionPointStore::instance().registerInjectionPoint(package, id,
                                                              parameters);
}
}
