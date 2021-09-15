
/**
  @file Injection.cpp
**/
#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/PlugStore.h"

using namespace VnV;

extern "C" {

VnV_Iterator _VnV_injectionPlug(VnV_Comm comm, const char* packageName,
                                const char* name, const char* fname, int line,
                                injectionDataCallback* callback,
                                int inputParameters, ...) {
  va_list argp;
  va_start(argp, inputParameters);
  NTV inputs = VariadicUtils::UnwrapVariadicArgs(argp, inputParameters);
  NTV outputs = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV_Iterator v = VnV::RunTime::instance().injectionPlug(
      comm, packageName, name, fname, line, callback, inputs, outputs);
  va_end(argp);
  return v;
}

int _VnV_injectionPlugRun(VnV_Iterator* iterator) {
  return VnV::RunTime::instance().injectionPlugRun(iterator);
}

void _VnV_registerInjectionPlug(const char* package, const char* id,
                                const char* parameters) {
  VnV::PlugStore::instance().registerPlug(package, id, parameters);
}
}
