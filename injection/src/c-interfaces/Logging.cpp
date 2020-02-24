
/**
  @file Logging.cpp
**/
#include "c-interfaces/Logging.h"
#include "base/Runtime.h"

void _VnV_registerLogLevel(const char *name, const char *color) {
    VnV::RunTime::instance().registerLogLevel(name,color);
}

void _VnV_Log(const char *p, const char *l , const char * format, ...) {
    va_list args;
    va_start(args,format);
    VnV::RunTime::instance().log(p,l,format,args);
    va_end(args);
}

int _VnV_BeginStage(const char *p, const char *format, ...) {
   va_list args;
   va_start(args,format);
   int ref = VnV::RunTime::instance().beginStage(p, format, args);
   va_end(args);
   return ref;
}

void _VnV_EndStage(int ref) {
   VnV::RunTime::instance().endStage(ref);
}
