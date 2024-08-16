#ifndef RUNTIME_INIT_INTERFACE_H
#define RUNTIME_INIT_INTERFACE_H
#ifndef __cplusplus

#include "c-interfaces/Wrappers.h"
#include "common-interfaces/all.h"
int VnV_init(const char *packageName, int *argc, char ***argv,
             initDataCallback icallback, registrationCallBack callback);


void VnV_finalize();

#ifndef VNV_OFF

#define INJECTION_INITIALIZE_C(PNAME, argc, argv, icallback)         \
  VnV_init(VNV_STR(PNAME), argc, argv, icallback,                    \
           VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#define INJECTION_INITIALIZE(PNAME, argc, argv)                      \
  INJECTION_INITIALIZE_C(PNAME, argc, argv, NULL)

#define INJECTION_FINALIZE(PNAME) VnV_finalize();

#else

#define INJECTION_INITIALIZE_C(...)
#define INJECTION_INITIALIZE_RAW_C(...)
#define INJECTION_INITIALIZE(...)
#define INJECTION_INITIALIZE_RAW(...)
#define INJECTION_FINALIZE(...)

#endif

#endif // RUNTIMEINTERFACE_H
#endif