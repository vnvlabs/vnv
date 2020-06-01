
/**
  @file Injection.cpp
**/
#include <stdarg.h>
#include "base/Runtime.h"
#include "c-interfaces/PackageName.h"
#include "c-interfaces/Wrappers.h"
#include "base/InjectionPointStore.h"
#include "base/Utilities.h"

using namespace VnV;

extern "C" {

void _VnV_injectionPoint(VnV_Comm comm, vnv_registration_function reg, const char *package,const char* id, injectionDataCallback *callback, ...) {
  va_list argp;
  va_start(argp, callback);
  NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().runTimePackageRegistration(package,reg);
  VnV::RunTime::instance().injectionPoint(comm,package, id, callback, map);
  va_end(argp);
}

void _VnV_injectionPoint_begin(VnV_Comm comm, vnv_registration_function reg, const char *package, const char* id, injectionDataCallback *callback, ...) {
  va_list argp;
  va_start(argp, callback);
  NTV map = VariadicUtils::UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().runTimePackageRegistration(package,reg);
  VnV::RunTime::instance().injectionPoint_begin(comm,package, id, callback, map);
  va_end(argp);
}

int _VnV_injectionPoint_end( const char * package, const char* id){
  VnV::RunTime::instance().injectionPoint_end(package, id);
  return true;
}

void _VnV_injectionPoint_loop(const char * package, const char* id, const char* stageId){
  VnV::RunTime::instance().injectionPoint_iter( package, id, stageId);
}

void _VnV_registerInjectionPoint(const char* package, const char * id, const char *parameters) {
    VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(package,id,parameters);
}


}
