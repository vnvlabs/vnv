
/**
  @file Injection.cpp
**/
#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/IteratorStore.h"

using namespace VnV;

extern "C" {

VnV_Iterator _VnV_injectionIteration(VnV_Comm comm, const char* packageName,
                                     const char* name, struct VnV_Function_Sig pretty,  const char* fname,
                                     int line, injectionDataCallback* callback,
                                     int once, int inputParameters, ...) {
  va_list argp;
  va_start(argp, inputParameters);
  NTV inputs = VariadicUtils::UnwrapVariadicArgs(argp, inputParameters);
  NTV outputs = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV_Iterator v = VnV::RunTime::instance().injectionIteration(
      comm, packageName, name, pretty, fname, line, callback, inputs, outputs, once);
  va_end(argp);
  return v;
}

int _VnV_injectionIterate(VnV_Iterator* iterator) {
  return VnV::RunTime::instance().injectionIterationRun(iterator);
}

void _VnV_registerInjectionIterator(const char* package, const char* id,
                                    const char* parameters) {
  VnV::IteratorStore::instance().registerIterator(package, id, parameters);
}
}
