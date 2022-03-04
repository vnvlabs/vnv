#ifndef RUNTIMEINTERFACE_H
#define RUNTIMEINTERFACE_H

#ifndef WITHOUT_VNV

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"

// All packages can register a function that returns a char* with the package
// json.

#  define REGISTER_FULL_JSON(PNAME, callback) \
    VnV_declarePackageJson(VNV_STR(PNAME), callback);

#  define INJECTION_INITIALIZE(PNAME, argc, argv, filename) \
    VnV_init(VNV_STR(PNAME), argc, argv, filename,          \
             VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#  define INJECTION_INITIALIZE_RAW(PNAME, argc, argv, inputjson) \
    VnV_init_raw(VNV_STR(PNAME), argc, argv, inputjson,          \
             VNV_REGISTRATION_CALLBACK_NAME(PNAME))

#  define INJECTION_FINALIZE(PNAME) VnV_finalize();

#  define DECLARESUBPACKAGE(NAME) INJECTION_REGISTRATION(NAME);

#  define REGISTERSUBPACKAGE(NAME)          \
    VnV_Register_Subpackage(#NAME, INJECTION_REGISTRATION_PTR(NAME));

#define RES(x) V
#define REGSUB(x) RES(x)
#define ESPI(...) FOR_EACH(DECLARESUBPACKAGE,__VA_ARGS__)
#define ESPR(...) FOR_EACH(REGISTERSUBPACKAGE,__VA_ARGS__)

#define INJECTION_EXECUTABLE_NOCLANG(package,...)\
    ESPI(__VA_ARGS__) \
    INJECTION_REGISTRATION(package) { \
       ESPR(__VA_ARGS__)              \
    }                                 \


// This doesn't expand to anything, just tells the VNV Registration generator to
// include a subpackage.
#  define INJECTION_SUBPACKAGE(PNAME, NAME)

typedef void (*registrationCallBack)();


VNVEXTERNC void VnV_Register_Subpackage(const char* Name,
                                        registrationCallBack callback);

typedef const char* (*vnvFullJsonStrCallback)();
VNVEXTERNC void VnV_declarePackageJson(const char* packageName,
                                       vnvFullJsonStrCallback callback);

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
VNVEXTERNC int VnV_init(const char* packageName, int* argc, char*** argv,
                        const char* filename, registrationCallBack callback);


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
VNVEXTERNC int VnV_init_raw(const char* packageName, int* argc, char*** argv,
                        const char* inputjson, registrationCallBack callback);

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

VNVEXTERNC void VnV_Registration_Info(const char* filename, int quit);

VNVEXTERNC void VnV_readFile(const char* reader, const char* filename);
VNVEXTERNC void VnV_readFileAndWalk(const char* reader, const char* filename,
                                    const char* package, const char* walker,
                                    const char* config);


VNVEXTERNC void* VnV_getOptionsObject(const char* package);

#define INJECTION_GET_CONFIG(PNAME) VnV_getOptionsObject(VNV_STR(PNAME));

#else  // WITHOUT_VNV
#  define VnV_init(...)
#  define VnV_finalize(...)
#  define VnV_runUnitTests()
#  define REGISTER_VNV_CALLBACK void __vnv_call_back_will_never_be_called

#endif

#endif  // RUNTIMEINTERFACE_H
