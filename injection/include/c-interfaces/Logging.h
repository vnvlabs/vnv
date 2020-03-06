#ifndef LOGGINGINTERFACE_H
#define LOGGINGINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

// IF LOGGING IS TURNED ON.
#ifndef WITHOUT_LOGGING

VNVEXTERNC void _VnV_registerLogLevel(const char *name, const char *color);
VNVEXTERNC void _VnV_Log(const char *p, const char * level, const char * message, ... ) __attribute__((format(printf,3,4)));
VNVEXTERNC int _VnV_BeginStage(const char *p, const char * message, ...) __attribute__((format(printf,2,3)));
VNVEXTERNC void _VnV_EndStage(int ref);

#define Register_Log_Level(NAME, COLOR) _VnV_registerLogLevel(NAME, COLOR);
#define VnV_Debug(...) _VnV_Log(PACKAGENAME_S,"DEBUG",__VA_ARGS__)
#define VnV_Warn(...) _VnV_Log(PACKAGENAME_S,"WARN",__VA_ARGS__)
#define VnV_Error(...) _VnV_Log(PACKAGENAME_S,"ERROR",__VA_ARGS__)
#define VnV_Info(...) _VnV_Log(PACKAGENAME_S,"INFO",__VA_ARGS__)
#define VnV_Log(level,...) _VnV_Log(PACKAGENAME_S,level,__VA_ARGS__)
#define VnV_BeginStage(...) _VnV_BeginStage(PACKAGENAME_S,__VA_ARGS__)
#define VnV_EndStage(ref) _VnV_EndStage(ref)

#else
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_Log(...)
#  define VnV_BeginStage(...) 1;
#  define VnV_EndStage(...)
#  define Register_Log_Level(...)

#endif //WITHOUT LOGGING.

#else //WITHOUT VNV
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_Log(...)
#  define VnV_BeginStage(...) 1;
#  define VnV_EndStage(...)
#  define Register_Log_Level(...)
#endif

#endif // LOGGINGINTERFACE_H
