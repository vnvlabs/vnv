
/**
  @file Injection.cpp
**/
#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/IteratorStore.h"

using namespace VnV;

namespace VnV {
DataCallback& DataCallback_wrapper(injectionDataCallback callback);
}

extern "C" {

VnV_Iterator _VnV_injectionIteration(VnV_Comm comm, const char* packageName, const char* name,
                                     struct VnV_Function_Sig pretty, const char* fname, int line,
                                     injectionDataCallback callback, int once, ...) {
  try {
    va_list argp;
    va_start(argp, once);
    NTV parameters = VariadicUtils::UnwrapVariadicArgs(argp);
    VnV_Iterator v = VnV::RunTime::instance().injectionIteration(comm, packageName, name, pretty, fname, line,
                                                                 DataCallback_wrapper(callback), parameters, once);
    va_end(argp);
    return v;
  } catch (std::exception& e) {
    assert(false && "This should never happen");
  }
}

int _VnV_injectionIterate(VnV_Iterator* iterator) {
  try {
    return VnV::RunTime::instance().injectionIterationRun(iterator);
  } catch (std::exception& e) {
    assert(false && "This should never happen");
  }
}

void _VnV_registerInjectionIterator(const char* package, const char* id, const char* parameters) {
  try {
    VnV::IteratorStore::instance().registerIterator(package, id, parameters);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Failed to register iterator %s:%s", package, id);
  }
}
}
