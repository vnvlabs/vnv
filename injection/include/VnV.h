

#ifndef VV_VNV_H
#define VV_VNV_H

#include "vnv-macros.h"

#define PACKAGENAME "VnV"

#  ifdef __cplusplus
#    define EXTERNC extern "C"
#  else
#    define EXTERNC
#  endif
/**
 * @brief VnV_injectionPoint
 * @param stageVal The stage of this injection point
 * @param id The id of the injection point
 * @param function The name of the function calling this injection point
 *
 * @arg Args The parameters of the injection point. These should be pairs of
 * string,void* where string is the class name, and void* is a pointer to a
 * class of that type. The final arguement should always be a string
 * "__VV_PARAMETERS_END__"
 *
 */
EXTERNC void _VnV_injectionPoint(const char *, int stageVal, const char* id, const char* function,...);
#define VnV_injectionPoint(stageVal, id, ...) _VnV_injectionPoint(PACKAGENAME, stageVal, id, __FUNCTION__, __VA_ARGS__ )

/**
 * @brief VnV_init
 * @param argc argc from the command line ( used in case of MPI_Init )
 * @param argv argv from the command line ( used in case of MPI_Init )
 * @param filename The configuration file name
 * @return todo.
 *
 * Initialize the VnV library. If this function is not called, no injection
 * point testing will take place.
 */
EXTERNC int VnV_init(int* argc, char*** argv, const char* filename);
/**
 * @brief VnV_finalize
 * @return todo
 *
 * Calls RunTime::instance().Finalize();
 */
EXTERNC int VnV_finalize();

/**
 * @brief VnV_runUnitTests
 * @return tod
 *
 * Calls RunTime::instance().runUnitTests().
 */
EXTERNC int VnV_runUnitTests();

#ifdef WITH_LOGGING

EXTERNC void _VnV_Debug(const char *p, const char * message, ...);
EXTERNC void _VnV_Warn(const char *p,const char * message, ...);
EXTERNC void _VnV_Error(const char *p, const char * message, ...);
EXTERNC void _VnV_Info(const char *p, const char * message, ...);
EXTERNC void _VnV_BeginStage(const char *p, const char * message, ...);
EXTERNC void _VnV_EndStage(const char *p, const char * message, ...);

#define VnV_Debug(...) _VnV_Debug(PACKAGENAME,__VA_ARGS__)
#define VnV_Warn(...) _VnV_Warn(PACKAGENAME,__VA_ARGS__)
#define VnV_Error(...) _VnV_Error(PACKAGENAME,__VA_ARGS__)
#define VnV_Info(...) _VnV_Info(PACKAGENAME,__VA_ARGS__)

#define VnV_BeginStage(...) _VnV_BeginStage(PACKAGENAME,__VA_ARGS__)
#define VnV_EndStage(...) _VnV_EndStage(PACKAGENAME,__VA_ARGS__)
#else
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_BeginStage(...)
#  define VnV_EndStage(...)
#endif

#undef EXTERNC
#endif
