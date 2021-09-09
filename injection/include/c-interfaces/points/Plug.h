#ifndef PLUGPOINTINTERFACE_H
#define PLUGPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)


# define INJECTION_FUNCTION_PLUG_C(PNAME, COMM, NAME, INPUTS, callback, ...)\
   VnV_Iterator VNV_JOIN(PNAME,_plug_,NAME) = _VnV_injectionPlug(COMM, PNANE, NAME, __FILE__, __LINE__, \
                    callback, INPUTS, EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S); \
   while(_VnV_injectionPlugRun(&VNV_JOIN(PNAME,_plug_,NAME)))

#  define INJECTION_FUNCTION_PLUG(PNAME, COMM, NAME, INPUTS, ...) \
    INJECTION_ITERATION_C(COMM, PNAME, NAME, INPUTS, NULL, __VA_ARGS__)

#  define Register_Injection_Plug(PNAME, NAME, PARAMETERS) _VnV_registerInjectionPlug(PNAME, NAME, PARAMETERS);


VNVEXTERNC VnV_Iterator  _VnV_injectionPlug(VnV_Comm comm, const char* packageName, const char* name, const char* fname, int line,
                                injectionDataCallback* callback, int inputParameters,...);

VNVEXTERNC int  _VnV_injectionPlugRun(VnV_Iterator *iterator);


VNVEXTERNC void _VnV_registerInjectionPlug(const char* packageName,
                                            const char* id,
                                            const char* parameters_str);




#else

#  define INJECTION_FUNCTION_PLUG_C(...)
#  define INJECTION_FUNCTION_PLUG(...)
#  define Register_Injection_Plug(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
