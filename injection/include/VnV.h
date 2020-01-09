

#ifndef VV_VNV_H
#define VV_VNV_H




#ifndef WITHOUT_VNV

#include "vnv-macros.h"

#ifndef PACKAGENAME
#  error "VnV: PACKAGENAME is not defined. Please Ensure a Macro called PACKAGENAME is defined before VnV.h is loaded"
#endif

#define VnV_E_STR(x) #x
#define VNV_STR(x) VnV_E_STR(x)
#define VNV_EX(x) x

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
typedef void (*registrationCallBack)();

#define VNV_REGISTRATION_CALLBACK_NAME __vnv_registration_callback__
#define VNV_REGISTRATION_CALLBACK_NAME_STR VNV_STR(VNV_REGISTRATION_CALLBACK_NAME)

#  ifdef __cplusplus
#    define VNV_EXTERNC extern "C"
#  else
#    define VNV_EXTERNC
#  endif


#define REGISTER_VNV_CALLBACK \
  VNV_EXTERNC void VNV_REGISTRATION_CALLBACK_NAME

#define PACKAGENAME_S  VNV_STR(PACKAGENAME)
#define VNV_END_PARAMETERS __vnv_end_parameters__
#define VNV_END_PARAMETERS_S VNV_STR(VNV_END_PARAMETERS)

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
EXTERNC void VnV_init(int* argc, char*** argv, const char* filename, registrationCallBack callback);
/**
 * @brief VnV_finalize
 * @return todo
 *
 * Calls RunTime::instance().Finalize();
 */
EXTERNC void VnV_finalize();
/**
 * @brief VnV_runUnitTests
 * @return tod
 *
 * Calls RunTime::instance().runUnitTests().
 */
EXTERNC void VnV_runUnitTests();

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
EXTERNC void _VnV_injectionPoint(const char * packageName, const char* id, const char* function, const char* file, int line, ...);
EXTERNC void _VnV_injectionPoint_begin(const char * packageName, const char* id, const char* function, const char* file, int line, ...);
EXTERNC void _VnV_injectionPoint_end(const char * packageName, const char* id, const char* function, const char* file, int line, ...);
EXTERNC void _VnV_injectionPoint_loop(const char * packageName, const char* id, const char* stageId, const char* function, const char* file, int line, ...);
EXTERNC void _VnV_registerInjectionPoint(const char* name, const char *json_str);

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
// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
    VNVPRAG(VnV, InjectionPoint, PACKAGENAME, NAME, __VA_ARGS__) \
   _VnV_injectionPoint(VNV_STR(PACKAGENAME),#NAME, __PRETTY_FUNCTION__,__FILE__,__LINE__, EVERY_SECOND(__VA_ARGS__) VNV_END_PARAMETERS_S);

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VNVPRAG(VnV, InjectionLoopBegin, PACKAGENAME, NAME, __VA_ARGS__) \
    _VnV_injectionPoint_begin(VNV_STR(PACKAGENAME), #NAME, __PRETTY_FUNCTION__,__FILE__,__LINE__, EVERY_SECOND(__VA_ARGS__) VNV_END_PARAMETERS_S);

// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME,...) \
    VNVPRAG(VnV, InjectionLoopEnd, PACKAGENAME, NAME, __VA_ARGS__) \
    _VnV_injectionPoint_end(VNV_STR(PACKAGENAME), #NAME, __PRETTY_FUNCTION__,__FILE__,__LINE__, EVERY_SECOND(__VA_ARGS__) VNV_END_PARAMETERS_S);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE,...) \
    VNVPRAG(VnV, InjectionLoopIter, PACKAGENAME, NAME, STAGE, __VA_ARGS__) \
    _VnV_injectionPoint_loop(VNV_STR(PACKAGENAME),#NAME,#STAGE, __PRETTY_FUNCTION__,__FILE__,__LINE__, EVERY_SECOND(__VA_ARGS__) VNV_END_PARAMETERS_S);

//REGISTER AN INJECTION POINT
#define Register_Injection_Point(NAME, CONFIG) \
    _VnV_registerInjectionPoint(NAME, CONFIG);


// IF LOGGING IS TURNED ON.
#ifndef WITHOUT_LOGGING

EXTERNC void _VnV_registerLogLevel(const char *name, const char *color);
EXTERNC void _VnV_Log(const char *p, const char * level, const char * message, ... ) __attribute__((format(printf,3,4)));
EXTERNC int _VnV_BeginStage(const char *p, const char * message, ...) __attribute__((format(printf,2,3)));
EXTERNC void _VnV_EndStage(int ref);

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

#undef EXTERNC

#else //If not without vnv
// No VnV, so just define out all injection point calls.

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Error(...)
#  define VnV_Info(...)
#  define VnV_Log(...)
#  define VnV_BeginStage(...) 1;
#  define VnV_EndStage(...)
#  define Register_Log_Level(...)

#  define VnV_init(...)
#  define VnV_finalize(...)
#  define VnV_runUnitTests()
# define REGISTER_VNV_CALLBACK void __vnv_call_back_will_never_be_called

#endif

#endif
