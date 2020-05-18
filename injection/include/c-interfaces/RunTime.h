#ifndef RUNTIMEINTERFACE_H
#define RUNTIMEINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"

#define REG_HELPER_(X,Y) X ## Y
#define REG_HELPER(X,Y) REG_HELPER_(X,Y)

#define VNV_REGISTRATION_CALLBACK_NAME REG_HELPER(__vnv_registration_callback__,PACKAGENAME)
#define VNV_GET_REGISTRATION "__vnv_registration_callback__"

#  ifdef __cplusplus
#   define INJECTION_REGISTRATION extern "C" void VNV_REGISTRATION_CALLBACK_NAME
#  else
#    define INJECTION_REGISTRATION void VNV_REGISTRATION_CALLBACK_NAME
#endif
#   define INJECTION_REGISTRATION_CALL VNV_REGISTRATION_CALLBACK_NAME();

// We are going to forward declare a registration function for any package that includes
// VnV.h. That way, the code will not compile if the registration function is missing.
INJECTION_REGISTRATION();


#define INJECTION_INITIALIZE(argc,argv,filename) \
    VnV_init(argc,argv,filename, VNV_REGISTRATION_CALLBACK_NAME);

#define INJECTION_FINALIZE() \
    VnV_finalize();


// This structure can be used to register VnV objects. Basically, the functions
// should return valid json strings, in the correct formats, for declaring objects. This
// is an alternative to using the static initialization approach in C++ codes, which results
// in objects being registered upon loading of the shared library. The goal will be to have
// the compiler generate the code (and json) required to populate this structure. In the VnV
// code, we will search the json for the export VnV_Registration_impl;
typedef void (*registrationCallBack)();

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
VNVEXTERNC void VnV_init(int* argc, char*** argv, const char* filename, registrationCallBack callback);
/**
 * @brief VnV_finalize
 * @return todo
 *
 * Calls RunTime::instance().Finalize();
 */
VNVEXTERNC void VnV_finalize();

/**
 * @brief VnV_runUnitTests
 * @return tod
 *
 * Calls RunTime::instance().runUnitTests().
*/

VNVEXTERNC void VnV_runUnitTests(VnV_Comm comm);

VNVEXTERNC void VnV_readFile(const char* filename);

#else // WITHOUT_VNV
#  define VnV_init(...)
#  define VnV_finalize(...)
#  define VnV_runUnitTests()
#  define REGISTER_VNV_CALLBACK void __vnv_call_back_will_never_be_called

#endif



#endif // RUNTIMEINTERFACE_H




