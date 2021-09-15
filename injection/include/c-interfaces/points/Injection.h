#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

// Injection Points

#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...)            \
    _VnV_injectionPoint(COMM, PNAME, NAME, __FILE__, __LINE__, callback, \
                        EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

#  define INJECTION_POINT(PNAME, COMM, NAME, ...) \
    INJECTION_POINT_C(COMM, PNAME, NAME, NULL, __VA_ARGS__)

#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...)             \
    _VnV_injectionPoint_begin(COMM, PNAME, NAME, __FILE__, __LINE__, callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

#  define INJECTION_LOOP_END(PNAME, NAME) \
    _VnV_injectionPoint_end(PNAME, NAME, __FILE__, __LINE__);

#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(PNAME, NAME, STAGE, __FILE__, __LINE__);

#  define INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, callback, \
                                       ...)                                   \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, __VA_ARGS__);                   \
    function(__VA_ARGS__);                                                    \
    INJECTION_LOOP_END(PNAME, NAME)

#  define INJECTION_FUNCTION_WRAPPER(PNAME, COMM, NAME, function, ...) \
    INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, NULL,    \
                                 __VA_ARGS__);

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    _VnV_registerInjectionPoint(PNAME, NAME, PARAMETERS);

VNVEXTERNC void _VnV_injectionPoint(VnV_Comm comm, const char* packageName,
                                    const char* id, const char* fname, int line,
                                    injectionDataCallback* callback, ...);

VNVEXTERNC void _VnV_injectionPoint_begin(VnV_Comm comm,
                                          const char* packageName,
                                          const char* id, const char* fname,
                                          int line,
                                          injectionDataCallback* callback, ...);

VNVEXTERNC void _VnV_injectionPoint_loop(const char* packageName,
                                         const char* id, const char* stageId,
                                         const char* fname, int line);

VNVEXTERNC int _VnV_injectionPoint_end(const char* packageName, const char* id,
                                       const char* fname, int line);

VNVEXTERNC void _VnV_registerInjectionPoint(const char* packageName,
                                            const char* id,
                                            const char* parameters_str);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_POINT_C(...)
#  define INJECTION_LOOP_BEGIN_C(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define INJECTION_FUNCTION_WRAPPER_C(...)
#  define INJECTION_FUNCTION_WRAPPER(...)
#  define Register_Injection_Point(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
