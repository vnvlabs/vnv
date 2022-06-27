
/**
  @file Injection.cpp
**/
#include <stdarg.h>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/InjectionPointStore.h"
#include "c-interfaces/Wrappers.h"
using namespace VnV;

namespace VnV {DataCallback& DataCallback_wrapper(injectionDataCallback* callback);}

extern "C" {

void _VnV_injectionPoint(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                         const char* fname, int line, injectionDataCallback* callback, ...) {
  try {
    va_list argp;
    va_start(argp, callback);
    NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
    VnV::RunTime::instance().injectionPoint(comm, package, id, pretty, fname, line, VnV::DataCallback_wrapper(callback), map);
    va_end(argp);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error launching injection point %s:%s", package, id);
  }
}

void _VnV_injectionPoint_begin(VnV_Comm comm, const char* package, const char* fname, int line, const char* id,
                               struct VnV_Function_Sig pretty, injectionDataCallback* callback, ...) {
  try {
    va_list argp;
    va_start(argp, callback);
    NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
    VnV::RunTime::instance().injectionPoint_begin(comm, package, id, pretty, fname, line, VnV::DataCallback_wrapper(callback), map);
    va_end(argp);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error launching injection point %s:%s", package, id);
  }
}

void _VnV_injectionPoint_loop(const char* package, const char* id, const char* stageId, const char* fname, int line) {
  try {
    VnV::RunTime::instance().injectionPoint_iter(package, id, stageId, fname, line);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error iterating injection point %s:%s", package, id);
  }
}

void _VnV_injectionPoint_end(const char* package, const char* id, const char* fname, int line) {
  try {
    VnV::RunTime::instance().injectionPoint_end(package, id, fname, line);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error ending injection point %s:%s", package, id);
  }
}

void _VnV_registerInjectionPoint(const char* package, const char* id, const char* parameters) {
  try {
    VnV::InjectionPointStore::instance().registerInjectionPoint(package, id, parameters);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error Registering injection point %s:%s", package, id);
  }
}
}
