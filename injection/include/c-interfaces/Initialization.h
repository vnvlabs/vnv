#ifndef RUNTIME_INIT_INTERFACE_H
#define RUNTIME_INIT_INTERFACE_H

#ifndef WITHOUT_VNV

#  ifndef __cplusplus

#    include "c-interfaces/Wrappers.h"
#    include "common-interfaces/Communication.h"
#    include "common-interfaces/PackageName.h"

#    define INJECTION_INITIALIZE_C(PNAME, argc, argv, icallback, filename) \
      VnV_init(VNV_STR(PNAME), argc, argv, filename, icallback, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#    define INJECTION_INITIALIZE_RAW_C(PNAME, argc, argv, icallback, inputjson) \
      VnV_init_raw(VNV_STR(PNAME), argc, argv, inputjson, icallback, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#    define INJECTION_INITIALIZE(PNAME, argc, argv, filename) \
      VnV_init(VNV_STR(PNAME), argc, argv, filename, NULL, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

// Initialize

#    define INJECTION_INITIALIZE_RAW(PNAME, argc, argv, inputjson) \
      VnV_init_raw(VNV_STR(PNAME), argc, argv, inputjson, NULL, VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#    define INJECTION_FINALIZE(PNAME) VnV_finalize();

/**
 * @brief VnV_init
 * @param argc argc from the command line ( used in case of MPI_Init )
 * @param argv argv from the command line ( used in case of MPI_Init )
 * @param filename The configuration file name
 * @return todo.
 *
 * Initialize the VnV library. If this function is not called, no injection
 * point testing will take place.
 */
int VnV_init(const char* packageName, int* argc, char*** argv, const char* filename, initDataCallback icallback,
             registrationCallBack callback);

/**
 * @brief VnV_init
 * @param argc argc from the command line ( used in case of MPI_Init )
 * @param argv argv from the command line ( used in case of MPI_Init )
 * @param filename The configuration file name
 * @return todo.
 *
 * Initialize the VnV library. If this function is not called, no injection
 * point testing will take place.
 */
int VnV_init_raw(const char* packageName, int* argc, char*** argv, const char* inputjson, initDataCallback icallback,
                 registrationCallBack callback);

void VnV_finalize();

#  endif

#else  // WITHOUT_VNV
#  define VnV_init(...)
#  define VnV_finalize(...)
#  define VnV_runUnitTests()
#  define REGISTER_VNV_CALLBACK void __vnv_call_back_will_never_be_called

#endif

#endif  // RUNTIMEINTERFACE_H
