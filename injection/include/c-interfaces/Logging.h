#ifndef LOGGINGINTERFACE_H
#define LOGGINGINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"

// IF LOGGING IS TURNED ON.
#  ifndef WITHOUT_LOGGING

VNVEXTERNC void _VnV_registerLogLevel(const char* packageName, const char* name,
                                      const char* color);

VNVEXTERNC void _VnV_Log(VnV_Comm comm, const char* p, const char* level,
                         const char* message, ...)
    __attribute__((format(printf, 4, 5)));

VNVEXTERNC int _VnV_BeginStage(VnV_Comm comm, const char* p,
                               const char* message, ...)
    __attribute__((format(printf, 3, 4)));

VNVEXTERNC void _VnV_EndStage(VnV_Comm comm, int ref);

// This macro allows packages to define custom logging levels. Here the
// name is the name of the log level (i.e., DEGUB,WARN,...). The color
// should be an ASCII color code sequence to determine the color formatting
// of the output when writing to a terminal that supports colors. In reality,
// the log statement is
// where COLOR_RESET is the ansi code to reset the terminal color to
// the standard terminal color. See wikipedia -- ANSI escape code -- to
// figure out the code you want to use.

// To use the custom log level, call VnV_Log(name, message, args...).
#    define INJECTION_LOGLEVEL(PNAME, NAME, COLOR)

#    define REGISTERLOGLEVEL(PNAME, NAME, COLOR) \
      _VnV_registerLogLevel(VNV_STR(PNAME), #NAME, #COLOR);

#    define VnV_Debug_MPI(PNAME, comm, ...) \
      _VnV_Log(comm, VNV_STR(PNAME), "DEBUG", __VA_ARGS__)
#    define VnV_Warn_MPI(PNAME, comm, ...) \
      _VnV_Log(comm, VNV_STR(PNAME), "WARN", __VA_ARGS__)
#    define VnV_Error_MPI(PNAME, comm, ...) \
      _VnV_Log(comm, VNV_STR(PNAME), "ERROR", __VA_ARGS__)
#    define VnV_Info_MPI(PNAME, comm, ...) \
      _VnV_Log(comm, VNV_STR(PNAME), "INFO", __VA_ARGS__)
#    define VnV_Log_MPI(PNAME, comm, level, ...) \
      _VnV_Log(comm, VNV_STR(PNAME), level, __VA_ARGS__)
#    define VnV_BeginStage_MPI(PNAME, comm, ...) \
      _VnV_BeginStage(comm, VNV_STR(PNAME), __VA_ARGS__)
#    define VnV_EndStage_MPI(comm, ref) _VnV_EndStage(comm, ref)

#    define VnV_Debug(PNAME, ...) \
      VnV_Debug_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_Warn(PNAME, ...) VnV_Warn_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_Error(PNAME, ...) \
      VnV_Error_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_Info(PNAME, ...) VnV_Info_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_Log(PNAME, ...) VnV_Log_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_BeginStage(PNAME, ...) \
      VnV_BeginStage_MPI(PNAME, VWORLD(PNAME), __VA_ARGS__)
#    define VnV_EndStage(PNAME, ...) \
      VnV_EndStage_MPI(VWORLD(PNAME), __VA_ARGS__)

#  else
#    define VnV_Debug(...)
#    define VnV_Warn(...)
#    define VnV_Error(...)
#    define VnV_Info(...)
#    define VnV_Log(...)
#    define VnV_BeginStage(...) 1;
#    define VnV_EndStage(...)
#    define Register_Log_Level(...)

#  endif  // WITHOUT LOGGING.

#else  // WITHOUT VNV
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_Log(...)
#  define VnV_BeginStage(...) 1;
#  define VnV_EndStage(...)
#  define Register_Log_Level(...)
#endif

#endif  // LOGGINGINTERFACE_H
