#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "common-interfaces/Communication.h"
#  include "common-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

#  define VNV_NOCALLBACK NULL

// Injection Points



#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, callback, ...)             \
    _VnV_injectionPoint_begin(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

#  define INJECTION_LOOP_END(PNAME, NAME, CALLBACK) \
    _VnV_injectionPoint_end(VNV_STR(PNAME), VNV_STR(NAME), __FILE__, __LINE__, CALLBACK);

#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE, CALLBACK) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), STAGE, __FILE__, __LINE__ , CALLBACK);


#  define INJECTION_POINT(PNAME, COMM, NAME, CALLBACK, ...) \
    INJECTION_LOOP_BEGIN(PNAME,COMM,NAME,CALLBACK,__VA_ARGS__)            \
    INJECTION_LOOP_END(PNAME,NAME,NULL,VNV_NOCALLBACK)

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    _VnV_registerInjectionPoint(PNAME, NAME, PARAMETERS);

VNVEXTERNC void _VnV_injectionPoint(VnV_Comm comm, const char* packageName,
                                    const char* id, struct VnV_Function_Sig pretty, const char* fname, int line,
                                    injectionDataCallback callback, ...);

VNVEXTERNC void _VnV_injectionPoint_begin(VnV_Comm comm,
                                          const char* packageName,
                                          const char* id, struct VnV_Function_Sig pretty, const char* fname,
                                          int line,
                                          injectionDataCallback callback, ...);

VNVEXTERNC void _VnV_injectionPoint_loop(const char* packageName,
                                         const char* id, const char* stageId,
                                         const char* fname, int line, injectionDataCallback callback );

VNVEXTERNC void _VnV_injectionPoint_end(const char* packageName, const char* id,
                                       const char* fname, int line, injectionDataCallback callback);

VNVEXTERNC void _VnV_registerInjectionPoint(const char* packageName,
                                            const char* id,
                                            const char* parameters_str);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define INJECTION_FUNCTION_WRAPPER_C(...)
#  define INJECTION_FUNCTION_WRAPPER(...)
#  define Register_Injection_Point(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
