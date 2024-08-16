#ifndef RUNTIME_INITCPP_INTERFACE_H
#define RUNTIME_INITCPP_INTERFACE_H

#include "common-interfaces/all.h"
#include "interfaces/IOutputEngine.h"


namespace VnV {

void defaultInitCallback(VnV_Comm comm, IOutputEngine* engine);
using InitDataCallback = std::function<void(VnV_Comm, IOutputEngine*)>;

int Init(const char* packageName, int* argc, char*** argv, InitDataCallback icallback, registrationCallBack callback);


void finalize();

}  // namespace VnV


#define INJECTION_INITIALIZE_C(PNAME, argc, argv, callback) \
  VnV::Init(VNV_STR(PNAME), argc, argv, callback, VNV_REGISTRATION_CALLBACK_NAME(PNAME))


#define INJECTION_INITIALIZE(PNAME, argc, argv) \
  INJECTION_INITIALIZE_C(PNAME, argc, argv, VnV::defaultInitCallback);


#define INJECTION_FINALIZE(PNAME) VnV::finalize();

#endif  // RUNTIMEINTERFACE_H
