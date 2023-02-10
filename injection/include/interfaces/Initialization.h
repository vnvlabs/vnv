#ifndef RUNTIME_INITCPP_INTERFACE_H
#define RUNTIME_INITCPP_INTERFACE_H

#include "common-interfaces/Communication.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/IOutputEngine.h"

#define INJECTION_INITIALIZE_C(PNAME, argc, argv, filename, callback) \
  VnV::Init(VNV_STR(PNAME), argc, argv, filename, callback, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#define INJECTION_INITIALIZE_RAW_C(PNAME, argc, argv, inputjson, callback) \
  VnV::Init_raw(VNV_STR(PNAME), argc, argv, inputjson, callback, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#define INJECTION_INITIALIZE(PNAME, argc, argv, filename) \
  INJECTION_INITIALIZE_C(PNAME, argc, argv, filename, VnV::defaultInitCallback);

#define INJECTION_INITIALIZE_RAW(PNAME, argc, argv, inputjson) \
  INJECTION_INITIALIZE_RAW_C(PNAME, argc, argv, inputjson, VnV::defaultInitCallback);

#define INJECTION_FINALIZE(PNAME) VnV::finalize();

namespace VnV {

void defaultInitCallback(VnV_Comm comm, IOutputEngine* engine);
using InitDataCallback = std::function<void(VnV_Comm, IOutputEngine*)>;

int Init(const char* packageName, int* argc, char*** argv, const char* filename, InitDataCallback icallback,
         registrationCallBack callback);

int Init_raw(const char* packageName, int* argc, char*** argv, const char* inputjson, InitDataCallback icallback,
             registrationCallBack callback);

void finalize();

}  // namespace VnV

#endif  // RUNTIMEINTERFACE_H
