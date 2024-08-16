#ifndef LOGGINGINTERFACE_H
#define LOGGINGINTERFACE_H

#include "common-interfaces/all.h"

VNVEXTERNC void _VnV_registerLogLevel(const char *packageName, const char *name, const char *color);

VNVEXTERNC void _VnV_Log(VnV_Comm comm, const char *p, const char *level, const char *message, ...) __attribute__((format(printf, 4, 5)));

VNVEXTERNC void _VnV_LogV(VnV_Comm comm, const char *p, const char *level, const char *message, va_list args);


#define INJECTION_LOGLEVEL(PNAME, NAME, COLOR)

#define REGISTERLOGLEVEL(PNAME, NAME, COLOR) _VnV_registerLogLevel(VNV_STR(PNAME), #NAME, #COLOR);

#define VnV_Debug_MPI(PNAME, comm, ...) _VnV_Log(comm, VNV_STR(PNAME), "DEBUG", __VA_ARGS__)
#define VnV_Warn_MPI(PNAME, comm, ...) _VnV_Log(comm, VNV_STR(PNAME), "WARN", __VA_ARGS__)
#define VnV_Error_MPI(PNAME, comm, ...) _VnV_Log(comm, VNV_STR(PNAME), "ERROR", __VA_ARGS__)
#define VnV_Info_MPI(PNAME, comm, ...)  _VnV_Log(comm, VNV_STR(PNAME), "INFO", __VA_ARGS__)
#define VnV_Log_MPI(PNAME, comm, level, ...) _VnV_Log(comm, VNV_STR(PNAME), level, __VA_ARGS__)

#define VnV_Debug_MPI_V(PNAME, comm, ...) _VnV_LogV(comm, VNV_STR(PNAME), "DEBUG", __VA_ARGS__)
#define VnV_Warn_MPI_V(PNAME, comm, ... ) _VnV_LogV(comm, VNV_STR(PNAME), "WARN", __VA_ARGS__)
#define VnV_Error_MPI_V(PNAME, comm, ...) _VnV_LogV(comm, VNV_STR(PNAME), "ERROR", __VA_ARGS__)
#define VnV_Info_MPI_V(PNAME, comm, ... ) _VnV_LogV(comm, VNV_STR(PNAME), "INFO", __VA_ARGS__)
#define VnV_Log_MPI_V(PNAME, comm, level, ...) _VnV_LogV(comm, VNV_STR(PNAME), level, __VA_ARGS__)

#define VnV_Debug(PNAME, ...) VnV_Debug_MPI(PNAME, VWORLD, __VA_ARGS__)
#define VnV_Warn(PNAME, ...) VnV_Warn_MPI(PNAME, VWORLD, __VA_ARGS__)
#define VnV_Error(PNAME, ...) VnV_Error_MPI(PNAME, VWORLD, __VA_ARGS__)
#define VnV_Info(PNAME, ...) VnV_Info_MPI(PNAME, VWORLD, __VA_ARGS__)
#define VnV_Log(PNAME, ...) VnV_Log_MPI(PNAME, VWORLD, __VA_ARGS__)

VNVEXTERNC void _VnV_registerFile(VnV_Comm comm, const char *packageName,
                                  const char *name, int input,
                                  const char *reader, const char *infilename,
                                  const char *outfilename);

#define ___INJECTION_FILE(PNAME, NAME, COMM, INPUT_BOOL, READER, INFILENAME, OUTFILENAME)  \
  _VnV_registerFile(COMM, VNV_STR(PNAME), #NAME, INPUT_BOOL, READER, INFILENAME, OUTFILENAME);

#define INJECTION_FILE(PNAME, COMM, NAME, INPUT, READER, FILENAME, OUTFILENAME) \
  ___INJECTION_FILE(PNAME,NAME,COMM,INPUT,READER,INFILENAME,OUTFILENAME)

#define INJECTION_INPUT_FILE_(PNAME, NAME, COMM, READER, INFILENAME, OUTFILENAME) \
  ___INJECTION_FILE(PNAME, NAME, COMM, 1, READER, INFILENAME, OUTFILENAME)

#define INJECTION_OUTPUT_FILE_(PNAME, NAME, COMM, READER, INFILENAME, OUTFILENAME) \
  ___INJECTION_FILE(PNAME, NAME, COMM, 0, READER, INFILENAME, OUTFILENAME)

#define INJECTION_INPUT_FILE(PNAME, NAME, COMM, INFILENAME, OUTFILENAME)       \
  INJECTION_INPUT_FILE_(PNAME, NAME, COMM, "auto", INFILENAME, OUTFILENAME)

#define INJECTION_OUTPUT_FILE(PNAME, NAME, COMM, INFILENAME, OUTFILENAME)      \
  INJECTION_OUTPUT_FILE_(PNAME, NAME, COMM, "auto", INFILENAME, OUTFILENAME)



#endif
