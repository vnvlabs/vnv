
#include <stdarg.h>
#include "vv-runtime.h"
#include "VnV.h"
#include "vv-injection.h"
using namespace VnV;

extern "C" {

void _VnV_injectionPoint(const char *package, const char* id, const char* function, const char* file, int line, ...) {
  va_list argp;
  va_start(argp, line);
  RunTime::instance().injectionPoint(package, id, function, file, line, argp);
  va_end(argp);
}
void _VnV_injectionPoint_begin(const char * package, const char* id, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  RunTime::instance().injectionPoint_begin(package, id, function, file, line, argp);
  va_end(argp);

}
void _VnV_injectionPoint_end(const char * package, const char* id, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  RunTime::instance().injectionPoint_end(package, id, function, file, line, argp);
  va_end(argp);

}
void _VnV_injectionPoint_loop(const char * package, const char* id, const char* stageId, const char* function, const char* file, int line, ...){
  va_list argp;
  va_start(argp, line);
  RunTime::instance().injectionPoint_iter(package, id, stageId, function, file, line, argp);
  va_end(argp);

}

void defaultRegistrationCallBack() {
    VnV_Info("The Registration Callback has not been set for this package.");
}

int VnV_init(int* argc, char*** argv, const char* filename, registrationCallBack callback) {
  RunTime::instance().Init(argc, argv, filename,&callback);
  return 1;
}

int VnV_finalize() {
  RunTime::instance().Finalize();
  return 1;
}

int VnV_runUnitTests() {
  RunTime::instance().runUnitTests();
  return 0;
}

void _VnV_registerInjectionPoint(const char* name, const char *json_str) {
   InjectionPointStore::getInjectionPointStore().registerInjectionPoint(name,json_str);
}

void _VnV_registerLogLevel(const char *name, const char *color) {
    RunTime::instance().registerLogLevel(name,color);
}

void _VnV_Log(const char *p, const char *l , const char * format, ...) {
    va_list args;
    va_start(args,format);
    RunTime::instance().log(p,l,format,args);
    va_end(args);
}

int _VnV_BeginStage(const char *p, const char *format, ...) {
   va_list args;
   va_start(args,format);
   int ref = RunTime::instance().beginStage(p, format, args);
   va_end(args);
   return ref;
}

void _VnV_EndStage(int ref) {
   RunTime::instance().endStage(ref);
}

}
