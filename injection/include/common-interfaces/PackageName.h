#ifndef PACKAGENAME_H
#define PACKAGENAME_H

#include <stdarg.h>

#define VNVPACKAGENAME VNV
#define VNVPACKAGENAME_S "VNV"

#define VnV_E_STR(x) #x
#define VNV_STR(x) VnV_E_STR(x)
#define VNV_EX(x) x
#define VNV_JOIN(x, y, z) x##y##z

#define VNVREGNAME __vnv_registration_callback__

#define VNV_DEFAULT_INPUT_FILE "__vnv_default_input_file__"

#if __cplusplus
#  define VNVEXTERNC extern "C"
#else
#  define VNVEXTERNC
#endif

typedef void (*registrationCallBack)();


#include "common-interfaces/prettyfunction.h"

// C Injection Macros.

#include "common-interfaces/foreach.h"

#define VNV_END_PARAMETERS __vnv_end_parameters__
#define VNV_END_PARAMETERS_S VNV_STR(VNV_END_PARAMETERS)

#define REG_HELPER_(X, Y) X##Y
#define REG_HELPER(X, Y) REG_HELPER_(X, Y)

#define VNV_REGISTRATION_CALLBACK_NAME(PNAME) REG_HELPER(VNVREGNAME, PNAME)
#define VNV_GET_REGISTRATION "__vnv_registration_callback__"

#ifdef __cplusplus
#  define INJECTION_REGISTRATION(PNAME)                    \
    extern "C" void __attribute__((visibility("default"))) \
    VNV_REGISTRATION_CALLBACK_NAME(PNAME)()

#  define FORTRAN_INJECTION_REGISTRATION(PNAME) \
    extern "C" registrationCallBack __attribute__((visibility("default"))) \
    vnv_fort_##PNAME##_x(){return INJECTION_REGISTRATION_PTR(PNAME);}   

#else

#  define INJECTION_REGISTRATION(PNAME)         \
    void __attribute__((visibility("default"))) \
    VNV_REGISTRATION_CALLBACK_NAME(PNAME)()

#  define FORTRAN_INJECTION_REGISTRATION(PNAME) \
    registrationCallBack __attribute__((visibility("default"))) \
    vnv_fort_##PNAME##_x(){return INJECTION_REGISTRATION_PTR(PNAME);}   

#endif



#define INJECTION_REGISTRATION_CALL(PNAME) VNV_REGISTRATION_CALLBACK_NAME(PNAME)
#define INJECTION_REGISTRATION_PTR(PNAME) &VNV_REGISTRATION_CALLBACK_NAME(PNAME)



// Allows the user to define a comment for any vnv injection object at some
// other place. Comments will be inserted based on the IMPORTANCE rating. The
// comment specified above a VNV Object has an importance rating of 0
#define INJECTION_COMMENT(PNAME, NAME, TYPE, IMPORTANT)

// Forward declare the VNV Registration Function.
INJECTION_REGISTRATION(VNVPACKAGENAME);



#define VNV_INCLUDED

#endif  // PACKAGENAME_H
