#ifndef PLUGPOINTINTERFACE_H
#define PLUGPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

#  define INJECTION_FUNCTION_PLUG_C(VAR, PNAME, COMM, NAME, callback, ...)                         \
    VnV_Iterator VAR = _VnV_injectionPlug(COMM, VNV_STR(PNANE), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                           EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);                       \
    while (_VnV_injectionPlugRun(&VAR))


#  define INJECTION_FUNCTION_PLUG(VAR, PNAME, COMM, NAME, ...) INJECTION_FUNCTION_PLUG_C(VAR, PNAME, COMM, NAME, NULL, __VA_ARGS__)


#  define Register_Injection_Plug(PNAME, NAME, PARAMETERS) _VnV_registerInjectionPlug(PNAME, NAME, PARAMETERS);

VNVEXTERNC VnV_Iterator _VnV_injectionPlug(VnV_Comm comm, const char* packageName, const char* name,
                                           struct VnV_Function_Sig pretty, const char* fname, int line,
                                           injectionDataCallback* callback, ...);

VNVEXTERNC int _VnV_injectionPlugRun(VnV_Iterator* iterator);

VNVEXTERNC void _VnV_registerInjectionPlug(const char* packageName, const char* id, const char* parameters_str);

#else

#  define INJECTION_FUNCTION_PLUG_C(...)
#  define INJECTION_FUNCTION_PLUG(...)
#  define Register_Injection_Plug(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
