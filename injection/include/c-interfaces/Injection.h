#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

#define DOIT(X)  #X, (void*)(&X),
#define EVERYONE(...) FOR_EACH(DOIT,__VA_ARGS__)

/**
 * Call the Runtime VnV_InjectionPoint function. This runs the tests.
*/




// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
   _VnV_injectionPoint(VNV_STR(PACKAGENAME),#NAME, EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);
VNVEXTERNC void _VnV_injectionPoint(const char * packageName, const char* id, ...);

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    _VnV_injectionPoint_begin(VNV_STR(PACKAGENAME), #NAME, EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);
VNVEXTERNC void _VnV_injectionPoint_begin(const char * packageName, const char* id, ...);


// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME) \
    _VnV_injectionPoint_end(VNV_STR(PACKAGENAME), #NAME );
VNVEXTERNC void _VnV_injectionPoint_end(const char * packageName, const char* id);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PACKAGENAME),#NAME,#STAGE);
VNVEXTERNC void _VnV_injectionPoint_loop(const char * packageName, const char* id, const char* stageId);

//REGISTER AN INJECTION POINT
#define Register_Injection_Point(CONFIG) \
    _VnV_registerInjectionPoint(CONFIG);
VNVEXTERNC void _VnV_registerInjectionPoint(const char *json_str);


#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif


#endif // INJECTIONPOINTINTERFACE_H
