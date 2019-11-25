
#include <stdarg.h>
#include "vv-runtime.h"
#include "VnV.h"
using namespace VnV;

extern "C" {

void _VnV_injectionPoint(const char *p, int stageVal, const char* id, const char* function,
                        ...) {
  va_list argp;
  va_start(argp, function);
  RunTime::instance().injectionPoint(p, stageVal, id, function, argp);
  va_end(argp);
}

int VnV_init(int* argc, char*** argv, const char* filename) {
  RunTime::instance().Init(argc, argv, filename);
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

#ifdef WITH_LOGGING

void _VnV_Warn(const char *p, const char * format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::WARN, format, args);
   va_end(args);
}

void _VnV_Info(const char *p, const char * format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::INFO, format, args);
   va_end(args);
}
void _VnV_Error(const char *p, const char * format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::ERROR, format, args);
   va_end(args);
}

void _VnV_Debug(const char *p, const char * format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::DEBUG, format, args);
   va_end(args);
}

void _VnV_BeginStage(const char *p, const char *format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::STAGE_START, format, args);
   va_end(args);
}

void _VnV_EndStage(const char *p, const char* format, ...) {
   va_list args;
   va_start(args,format);
   RunTime::instance().log(p,LogLevel::STAGE_END, format, args);
   va_end(args);
}

}

#endif
