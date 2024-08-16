#ifndef RUNTIMEINTERFACE_H
#define RUNTIMEINTERFACE_H


#  include "common-interfaces/all.h"

typedef void (*registrationCallBack)();

VNVEXTERNC void VnV_Register_Subpackage(const char* Name, registrationCallBack callback);

typedef const char* (*vnvFullJsonStrCallback)();
VNVEXTERNC void VnV_declarePackageJson(const char* packageName, vnvFullJsonStrCallback callback);

VNVEXTERNC void VnV_runUnitTests(VnV_Comm comm);

VNVEXTERNC void VnV_Registration_Info(const char* filename, int quit);


// All packages can register a function that returns a char* with the package
// json.
#ifndef VNV_OFF

#  define REGISTER_FULL_JSON(PNAME, callback) VnV_declarePackageJson(VNV_STR(PNAME), callback);

#  define DECLARESUBPACKAGE(NAME) INJECTION_REGISTRATION(NAME);

#  define REGISTERSUBPACKAGE(NAME) VnV_Register_Subpackage(#  NAME, INJECTION_REGISTRATION_PTR(NAME));

#  define RES(x) V
#  define REGSUB(x) RES(x)
#  define ESPI(...) FOR_EACH(DECLARESUBPACKAGE, __VA_ARGS__)
#  define ESPR(...) FOR_EACH(REGISTERSUBPACKAGE, __VA_ARGS__)

#  define INJECTION_EXECUTABLE_NOCLANG(package, ...) \
    ESPI(__VA_ARGS__)                                \
    INJECTION_REGISTRATION(package) { ESPR(__VA_ARGS__) }

#  define INJECTION_LIBRARY_NOCLANG(package, ...) \
    ESPI(__VA_ARGS__)                             \
    INJECTION_REGISTRATION(package) { ESPR(__VA_ARGS__) }

// This doesn't expand to anything, just tells the VNV Registration generator to
// include a subpackage.
#  define INJECTION_SUBPACKAGE(PNAME, NAME)


#  define INJECTION_CODEBLOCK_START(PNAME, NAME)
#  define INJECTION_CODEBLOCK_END(PNAME, NAME)

#else 
#  define VnV_init(...)
#  define VnV_finalize(...)
#  define VnV_runUnitTests()
#  define REGISTER_VNV_CALLBACK void __vnv_call_back_will_never_be_called

#endif

#endif  // RUNTIMEINTERFACE_H
