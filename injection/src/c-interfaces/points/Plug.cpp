
/**
  @file Injection.cpp
**/
#include "interfaces/points/Plug.h"

#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/PlugStore.h"

using namespace VnV;
namespace VnV {
DataCallback DataCallback_wrapper(injectionDataCallback callback);
}


extern "C" {

VnV_Iterator _VnV_injectionPlug(VnV_Comm comm, const char* packageName, const char* name,
                                struct VnV_Function_Sig pretty, const char* fname, int line,
                                injectionDataCallback callback, ...) {
  try {
    va_list argp;
    va_start(argp, callback);
    NTV parameters = VariadicUtils::UnwrapVariadicArgs(argp);
    va_end(argp);
    VnV_Iterator v = VnV::RunTime::instance().injectionPlug(comm, packageName, name, pretty, fname, line,
                                                            DataCallback_wrapper(callback), parameters);
    return v;

  } catch (std::exception& e) {
    assert(false && "This should never happen");
    VnV_Error(VNVPACKAGENAME, "Failed to initialize Plug %s:%s", packageName, name);
  }
}

int _VnV_injectionPlugRun(VnV_Iterator* iterator) {
  try {
    return VnV::RunTime::instance().injectionPlugRun(iterator);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Failed to Run Plug. Running default instead.");
  }
  return 0;
}

void _VnV_registerInjectionPlug(const char* package, const char* id, const char* parameters) {
  try {
    VnV::PlugStore::instance().registerPlug(package, id, parameters);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Failed to register Plug %s:%s", package, id);
  }
}
}
