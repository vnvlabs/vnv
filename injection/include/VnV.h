

#ifndef VV_VNV_H
#define VV_VNV_H

#ifndef WITHOUT_VNV

#include "vnv-macros.h"
#define PACKAGENAME VnV

#define VNV_STR(x) #x

#  ifdef __cplusplus
#    define EXTERNC extern "C"
#  else
#    define EXTERNC
#  endif

// This structure can be used to register VnV objects. Basically, the functions
// should return valid json strings, in the correct formats, for declaring objects. This
// is an alternative to using the static initialization approach in C++ codes, which results
// in objects being registered upon loading of the shared library. The goal will be to have
// the compiler generate the code (and json) required to populate this structure. In the VnV
// code, we will search the json for the export VnV_Registration_impl;
#define VNV_REG_OBJ __vnv_registration_object
#define VNV_REG_STR VNV_STR(VNV_REG_OBJ)
#define PACKAGENAME_S VNV_STR(PACKAGENAME)

struct VnV_Registration {
    char* (*info)(void);
};

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
/**
 * Helper Define
 */
#define H1(prefix, ...) VNV_STR(prefix #__VA_ARGS__)

/**
 * Writes a Valid _Pragma statement
 */
#define VNVPRAG(prefix,...) _Pragma(H1(prefix, __VA_ARGS__))
/**
 * Call the Runtime VnV_InjectionPoint function. This runs the tests.
*/
#define INJECTION_POINT_(NAME, STAGE, ...)        \
    _VnV_injectionPoint(PACKAGENAME_S,STAGE,#NAME, __FUNCTION__, EVERY_SECOND(__VA_ARGS__) "VV_END_PARAMETERS");

/**
 * The Main Injection point function. This function does two things. First, it writes a #pragma that can
 * be used by the compiler to detect and log injection points. Second, it calls the runtime injection
 * point code, through the INJECTION_POINT_ macro.
 */
#define INJECTION_POINT(...) \
    VNVPRAG(VnV InjectionPoint, PACKAGENAME, __VA_ARGS__) \
    INJECTION_POINT_(__VA_ARGS__)


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

#ifndef WITHOUT_LOGGING

EXTERNC void _VnV_Debug(const char *p, const char * message, ...);
EXTERNC void _VnV_Warn(const char *p,const char * message, ...);
EXTERNC void _VnV_Error(const char *p, const char * message, ...);
EXTERNC void _VnV_Info(const char *p, const char * message, ...);
EXTERNC void _VnV_BeginStage(const char *p, const char * message, ...);
EXTERNC void _VnV_EndStage(const char *p, const char * message, ...);

#define VnV_Debug(...) _VnV_Debug(PACKAGENAME_S,__VA_ARGS__)
#define VnV_Warn(...) _VnV_Warn(PACKAGENAME_S,__VA_ARGS__)
#define VnV_Error(...) _VnV_Error(PACKAGENAME_S,__VA_ARGS__)
#define VnV_Info(...) _VnV_Info(PACKAGENAME_S,__VA_ARGS__)

#define VnV_BeginStage(...) _VnV_BeginStage(PACKAGENAME_S,__VA_ARGS__)
#define VnV_EndStage(...) _VnV_EndStage(PACKAGENAME_S,__VA_ARGS__)

#else
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_BeginStage(...)
#  define VnV_EndStage(...)

#endif //WITHOUT LOGGING.

#undef EXTERNC

#else //If not without vnv
// No VnV, so just define out all injection point calls.

#  define INJECTION_POINT(...)
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_BeginStage(...)
#  define VnV_EndStage(...)

#endif

#endif
