#ifndef LOGGINGINTERFACE_H
#define LOGGINGINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

// IF LOGGING IS TURNED ON.
#ifndef WITHOUT_LOGGING

VNVEXTERNC void _VnV_registerLogLevel(const char *name, const char *color);
VNVEXTERNC void _VnV_Log(VnV_Comm comm, const char *p, const char * level, const char * message, ... ) __attribute__((format(printf,4,5)));
VNVEXTERNC int _VnV_BeginStage(VnV_Comm comm, const char *p, const char * message, ...) __attribute__((format(printf,3,4)));
VNVEXTERNC void _VnV_EndStage(VnV_Comm comm, int ref);

#define Register_Log_Level(NAME, COLOR) _VnV_registerLogLevel(NAME, COLOR);
#define VnV_Debug_MPI(comm,...) _VnV_Log(comm,PACKAGENAME_S,"DEBUG",__VA_ARGS__)
#define VnV_Warn_MPI(comm,...) _VnV_Log(comm,PACKAGENAME_S,"WARN",__VA_ARGS__)
#define VnV_Error_MPI(comm,...) _VnV_Log(comm,PACKAGENAME_S,"ERROR",__VA_ARGS__)
#define VnV_Info_MPI(comm,...) _VnV_Log(comm,PACKAGENAME_S,"INFO",__VA_ARGS__)
#define VnV_Log_MPI(level,...) _VnV_Log(comm,PACKAGENAME_S,level,__VA_ARGS__)
#define VnV_BeginStage_MPI(comm,...) _VnV_BeginStage(comm,PACKAGENAME_S,__VA_ARGS__)
#define VnV_EndStage_MPI(comm,ref) _VnV_EndStage(comm,ref)

#define VnV_Debug(...) VnV_Debug_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_Warn(...) VnV_Warn_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_Error(...) VnV_Error_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_Info(...) VnV_Info_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_Log(...) VnV_Log_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_BeginStage(...) VnV_BeginStage_MPI(VnV_Comm_Self,__VA_ARGS__)
#define VnV_EndStage(...) VnV_EndStage_MPI(VnV_Comm_Self,__VA_ARGS__)


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
