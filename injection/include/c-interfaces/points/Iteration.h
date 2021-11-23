#ifndef ITERATIONPOINTINTERFACE_H
#define ITERATIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

#  define INJECTION_ITERATION_C(PNAME, COMM, NAME, ONCE, INPUTS, callback,    \
                                ...)                                          \
    VnV_Iterator VNV_JOIN(PNAME, _iterator_, NAME) = _VnV_injectionIteration( \
        COMM, PNANE, NAME, VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, ONCE, INPUTS,        \
        EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);                          \
    while (_VnV_injectionIterate(&VNV_JOIN(PNAME, _iterator_, NAME)))

#  define INJECTION_ITEARTION(PNAME, COMM, NAME, ONCE, INPUTS, ...) \
    INJECTION_ITERATION_C(COMM, PNAME, NAME, ONCE, INPUTS, NULL, __VA_ARGS__)

#  define Register_Injection_Iterator(PNAME, NAME, PARAMETERS) \
    _VnV_registerInjectionIterator(PNAME, NAME, PARAMETERS);

VNVEXTERNC VnV_Iterator _VnV_injectionIteration(
    VnV_Comm comm, const char* packageName, const char* name, struct VnV_Function_Sig pretty, const char fname,
    int line, injectionDataCallback* callback, int once, int inputParameters,
    ...);

VNVEXTERNC int _VnV_injectionIterate(VnV_Iterator* iterator);

VNVEXTERNC void _VnV_registerInjectionIterator(const char* packageName,
                                               const char* id,
                                               const char* parameters_str);

#else

#  define INJECTION_ITERATION_C(...)
#  define INJECTION_ITERATION(...)
#  define Register_Injection_Iterator(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
