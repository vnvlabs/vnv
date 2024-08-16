#ifndef PACKAGENAME_H
#define PACKAGENAME_H

#include <stdarg.h>
#include "common-interfaces/internal/prettyfunction.h"
#include "common-interfaces/internal/foreach.h"
#include "common-interfaces/internal/vmpi.h"

#define VNVPACKAGENAME VNV
#define VNVPACKAGENAME_S "VNV"

#define VnV_E_STR(x) #x
#define VNV_STR(x) VnV_E_STR(x)
#define VNV_EX(x) x
#define VNV_JOIN(x, y, z) x##y##z

#define VNVREGNAME __vnv_registration_callback__

#define VNV_DEFAULT_INPUT_FILE_OFF "__vnvoff__"
#define VNV_DEFAULT_INPUT_FILE_ON "__vnvon__"
#define VNV_DEFAULT_INPUT_FILE_ALL "__vnvall__"

#if __cplusplus
#  define VNVEXTERNC extern "C"
#else
#  define VNVEXTERNC
#endif

// MPI is a required dependency 
//of the VnV library. But, it might not be a depdendency of an
//application using it. The VnV Comm wrapper makes sure mpi is 
//not required in applications using the toolkit. 
struct VnV_Comm_ {
    void* data;
}; 

typedef struct VnV_Comm_ VnV_Comm;

VNVEXTERNC VnV_Comm VnV_Comm_Self();
VNVEXTERNC VnV_Comm VnV_Comm_World();

#define VSELF VnV_Comm_Self()
#define VWORLD VnV_Comm_World()

#ifndef WITHOUT_MPI

//If we have mpi, then we can create custom VnV_Comm objects
//from MPI_Comm objects. 
VNVEXTERNC VnV_Comm VnV_Comm_Cust(MPI_Comm comm);
#define VCUST(comm) VnV_Comm_Cust(comm)

#endif


typedef void (*registrationCallBack)();



#define VNV_END_PARAMETERS __vnv_end_parameters__
#define VNV_END_PARAMETERS_S VNV_STR(VNV_END_PARAMETERS)
#define REG_HELPER_(X, Y) X##Y
#define REG_HELPER(X, Y) REG_HELPER_(X, Y)

#define VNV_GET_REGISTRATION "__vnv_registration_callback__"
#define VNV_REGISTRATION_CALLBACK_NAME(PNAME) REG_HELPER(VNVREGNAME, PNAME)
#define INJECTION_REGISTRATION_PTR(PNAME) &VNV_REGISTRATION_CALLBACK_NAME(PNAME)


#ifdef __cplusplus

#  define INJECTION_REGISTRATION(PNAME) \
    extern "C" void __attribute__((visibility("default"))) VNV_REGISTRATION_CALLBACK_NAME(PNAME)()

#  define FORTRAN_INJECTION_REGISTRATION(PNAME)                                                     \
    extern "C" registrationCallBack __attribute__((visibility("default"))) vnv_fort_##PNAME##_x() { \
      return INJECTION_REGISTRATION_PTR(PNAME);                                                     \
    }

#else

#  define INJECTION_REGISTRATION(PNAME) \
    void __attribute__((visibility("default"))) VNV_REGISTRATION_CALLBACK_NAME(PNAME)()

#  define FORTRAN_INJECTION_REGISTRATION(PNAME)                                          \
    registrationCallBack __attribute__((visibility("default"))) vnv_fort_##PNAME##_x() { \
      return INJECTION_REGISTRATION_PTR(PNAME);                                          \
    }

#endif

#define INJECTION_EXECUTABLE(package, input) INJECTION_REGISTRATION(package);
#define INJECTION_LIBRARY(package) INJECTION_REGISTRATION(package);
#define INJECTION_PLUGIN(package) INJECTION_REGISTRATION(package);

// Forward declare the VNV Registration Function.
INJECTION_REGISTRATION(VNVPACKAGENAME);

#define VNV_INCLUDED

#endif  // PACKAGENAME_H
