#ifndef ITERATIONPOINTINTERFACE_H
#define ITERATIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "common-interfaces/Communication.h"
#  include "common-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

#  define INJECTION_ITERATION_C(VAR, PNAME, COMM, NAME, ONCE, callback, ...)                                  \
    VnV_Iterator VAR =                                                     \
        _VnV_injectionIteration(COMM, VNV_STR(PNANE), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, ONCE, \
                                EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);                             \
    while (_VnV_injectionIterate(&VAR, _iterator_, NAME)))

#  define INJECTION_ITERATION(VAR, PNAME, COMM, NAME, ONCE, ...) \
    INJECTION_ITERATION_C(VAR, COMM, PNAME, NAME, ONCE, NULL, __VA_ARGS__)

#  define Register_Injection_Iterator(PNAME, NAME, PARAMETERS) _VnV_registerInjectionIterator(PNAME, NAME, PARAMETERS);

VNVEXTERNC VnV_Iterator _VnV_injectionIteration(VnV_Comm comm, const char* packageName, const char* name,
                                                struct VnV_Function_Sig pretty, const char fname, int line,
                                                injectionDataCallback callback, int once, ...);

VNVEXTERNC int _VnV_injectionIterate(VnV_Iterator* iterator);

VNVEXTERNC void _VnV_registerInjectionIterator(const char* packageName, const char* id, const char* parameters_str);

#else

#  define INJECTION_ITERATION_C(...)
#  define INJECTION_ITERATION(...)
#  define Register_Injection_Iterator(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
