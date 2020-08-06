#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

typedef void (*vnv_registration_function)();

VNVEXTERNC int _VnV_injectionPoint_end(const char* packageName, const char* id);
VNVEXTERNC void _VnV_injectionPoint_loop(const char* packageName,
                                         const char* id, const char* stageId);
VNVEXTERNC void _VnV_registerInjectionPoint(const char* packageName,
                                            const char* id,
                                            const char* parameters_str);
VNVEXTERNC void _VnV_injectionPoint_begin(VnV_Comm comm,
                                          const char* packageName,
                                          const char* id,
                                          injectionDataCallback* callback, ...);
VNVEXTERNC void _VnV_injectionPoint(VnV_Comm comm, const char* packageName,
                                    const char* id,
                                    injectionDataCallback* callback, ...);

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

// Macro for an injection point with a callback
#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...)          \
    _VnV_injectionPoint(COMM, VNV_STR(PNAME), VNV_STR(NAME), callback, \
                        EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

// Injection point without a data callback.
#  define INJECTION_POINT(PNAME, COMM, NAME, ...) \
    INJECTION_POINT_C(COMM, PNAME, NAME, NULL, __VA_ARGS__)

// BEGIN A LOOPED INJECTION POINT with a callback
#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...)           \
    _VnV_injectionPoint_begin(COMM, VNV_STR(PNAME), VNV_STR(NAME), callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

// Begin a looped injection point without a data callback.
#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

// END A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_END(PNAME, NAME) \
    _VnV_injectionPoint_end(VNV_STR(PNAME), #NAME);

#  define INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, callback, \
                                       ...)                                   \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, __VA_ARGS__);                   \
    function(__VA_ARGS__);                                                    \
    INJECTION_LOOP_END(PNAME, NAME)

#  define INJECTION_FUNCTION_WRAPPER(PNAME, COMM, NAME, function, ...) \
    INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, NULL,    \
                                 __VA_ARGS__);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), #STAGE);

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    _VnV_registerInjectionPoint(VNV_STR(PNAME), VNV_STR(NAME), PARAMETERS);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
