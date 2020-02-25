#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X)FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X)FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X)FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...) WHAT(X)FE_8(WHAT, __VA_ARGS__)
#define FE_10(WHAT, X, ...) WHAT(X)FE_9(WHAT, __VA_ARGS__)
#define FE_11(WHAT, X, ...) WHAT(X)FE_10(WHAT, __VA_ARGS__)
#define FE_12(WHAT, X, ...) WHAT(X)FE_11(WHAT, __VA_ARGS__)
#define FE_13(WHAT, X, ...) WHAT(X)FE_12(WHAT, __VA_ARGS__)
#define FE_14(WHAT, X, ...) WHAT(X)FE_13(WHAT, __VA_ARGS__)
#define FE_15(WHAT, X, ...) WHAT(X)FE_14(WHAT, __VA_ARGS__)
#define FE_16(WHAT, X, ...) WHAT(X)FE_15(WHAT, __VA_ARGS__)
#define FE_17(WHAT, X, ...) WHAT(X)FE_16(WHAT, __VA_ARGS__)
#define FE_18(WHAT, X, ...) WHAT(X)FE_17(WHAT, __VA_ARGS__)
#define FE_19(WHAT, X, ...) WHAT(X)FE_18(WHAT, __VA_ARGS__)
#define FE_20(WHAT, X, ...) WHAT(X)FE_19(WHAT, __VA_ARGS__)
#define FE_21(WHAT, X, ...) WHAT(X)FE_20(WHAT, __VA_ARGS__)
#define FE_22(WHAT, X, ...) WHAT(X)FE_21(WHAT, __VA_ARGS__)
#define FE_23(WHAT, X, ...) WHAT(X)FE_22(WHAT, __VA_ARGS__)
#define FE_24(WHAT, X, ...) WHAT(X)FE_23(WHAT, __VA_ARGS__)
#define FE_25(WHAT, X, ...) WHAT(X)FE_24(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,NAME,...) NAME

#define FOR_EACH(action,...) \
  GET_MACRO(_0,__VA_ARGS__, FE_25,FE_24,FE_23,FE_22,FE_21,FE_20,FE_19,FE_18,FE_17,FE_16,FE_15,FE_14,FE_13,FE_12,FE_11,FE_10,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__)


// Ifdef on  CPP, primarily to get rid of the old-style cast warnings.
#ifndef __cplusplus
#define DOIT(X)  #X, (void*)(&X),
#else
#define DOIT(X) #X, static_cast<void*>(&X),
#endif


#define EVERYONE(...) FOR_EACH(DOIT,__VA_ARGS__)
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

#define VNV_END_PARAMETERS __vnv_end_parameters__
#define VNV_END_PARAMETERS_S VNV_STR(VNV_END_PARAMETERS)

// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
    VNVPRAG(VnV, InjectionPoint, PACKAGENAME, NAME, __VA_ARGS__) \
   _VnV_injectionPoint(VNV_STR(PACKAGENAME),#NAME,EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VNVPRAG(VnV, InjectionLoopBegin, PACKAGENAME, NAME, __VA_ARGS__) \
    _VnV_injectionPoint_begin(VNV_STR(PACKAGENAME), #NAME, EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME) \
    VNVPRAG(VnV, InjectionLoopEnd, PACKAGENAME, NAME) \
    _VnV_injectionPoint_end(VNV_STR(PACKAGENAME), #NAME );

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE) \
    VNVPRAG(VnV, InjectionLoopEnd, PACKAGENAME, NAME) \
    _VnV_injectionPoint_loop(VNV_STR(PACKAGENAME),#NAME,#STAGE);

//REGISTER AN INJECTION POINT
#define Register_Injection_Point(CONFIG) \
    _VnV_registerInjectionPoint(CONFIG);

#if __cplusplus
#define EXTERNC extern "C"
#else
   #define EXTERNC
#endif

EXTERNC void _VnV_injectionPoint(const char * packageName, const char* id, ...);
EXTERNC void _VnV_injectionPoint_begin(const char * packageName, const char* id,...);
EXTERNC void _VnV_injectionPoint_end(const char * packageName, const char* id);
EXTERNC void _VnV_injectionPoint_loop(const char * packageName, const char* id, const char* stageId);
EXTERNC void _VnV_registerInjectionPoint(const char *json_str);
#undef EXTERNC

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif


#endif // INJECTIONPOINTINTERFACE_H
