#ifndef INJECTION_CPP_H
#define INJECTION_CPP_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

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
    VnV::CppInjectionPoint(VNV_STR(PACKAGENAME),#NAME,"Single",__VA_ARGS__);

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VNVPRAG(VnV, InjectionLoopBegin, PACKAGENAME, NAME, __VA_ARGS__) \
    VnV::CppInjectionPoint(VNV_STR(PACKAGENAME), #NAME,"Begin", __VA_ARGS__);

// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME) \
    VNVPRAG(VnV, InjectionLoopEnd, PACKAGENAME, NAME) \
    VnV::CppInjectionPoint(VNV_STR(PACKAGENAME), #NAME,"End" );

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE) \
    VNVPRAG(VnV, InjectionLoopEnd, PACKAGENAME, NAME) \
    VnV::CppInjectionPoint(VNV_STR(PACKAGENAME),#NAME, "Iter",#STAGE);

//REGISTER AN INJECTION POINT
#define Register_Injection_Point(CONFIG) \
    _VnV_registerInjectionPoint(CONFIG);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif


#endif // INJECTIONPOINTINTERFACE_H


#endif // INJECTION_H
