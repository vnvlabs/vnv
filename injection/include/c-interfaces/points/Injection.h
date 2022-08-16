#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include "common-interfaces/Communication.h"
#  include "common-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"

#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

// Injection Points
// "..." refers to "variadic arguments" throughout this file
//   variadic args enable the user to input an arbitrary number of arguments 
//   to the respective "variadic function" that has "..." as a parameter

#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...) \
    _VnV_injectionPoint(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                        EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

//special case of INJECTION_POINT_C where ...
//  callback=NULL
//  ...=__VA_ARGS__, where "__VA_ARGS__" represents the variadic arguments that the user inputs as an argument for the "..." parameter
#  define INJECTION_POINT(PNAME, COMM, NAME, ...) \
    INJECTION_POINT_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...) \
    _VnV_injectionPoint_begin(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

#  define INJECTION_LOOP_END(PNAME, NAME) \
    _VnV_injectionPoint_end(VNV_STR(PNAME), VNV_STR(NAME), __FILE__, __LINE__);

#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), VNV_STR(STAGE), __FILE__, __LINE__);

//FIXME remove ) after STAGE ?
#  define INJECTION_LOOP_ITER_D(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), STAGE), __FILE__, __LINE__);


#  define INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, callback, \
                                       ...)                                   \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, __VA_ARGS__);                   \
    function(__VA_ARGS__);                                                    \
    INJECTION_LOOP_END(PNAME, NAME)

#  define INJECTION_FUNCTION_WRAPPER(PNAME, COMM, NAME, function, ...) \
    INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, NULL,    \
                                 __VA_ARGS__);

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    _VnV_registerInjectionPoint(PNAME, NAME, PARAMETERS);

// VNVEXTERNC is a VnV macro that substitutes   extern "C"  for VNVEXTERNC if the filetype is .cpp
//   if the filetype is not .cpp, then it substitutes whitespace for VNVEXTERNC
// When you state that a function has extern "C" ilnkage in C++, 
//   the C++ compiler does not add argument/parameter type information 
//   to the name used for linkage
// https://stackoverflow.com/questions/1041866/what-is-the-effect-of-extern-c-in-c
VNVEXTERNC void _VnV_injectionPoint(VnV_Comm comm, 
                                    const char* packageName,
                                    const char* id, 
                                    struct VnV_Function_Sig pretty, 
                                    const char* fname, 
                                    int line,
                                    injectionDataCallback* callback, ...);

VNVEXTERNC void _VnV_injectionPoint_begin(VnV_Comm comm,
                                          const char* packageName,
                                          const char* id, struct VnV_Function_Sig pretty, const char* fname,
                                          int line,
                                          injectionDataCallback* callback, ...);

VNVEXTERNC void _VnV_injectionPoint_loop(const char* packageName,
                                         const char* id, const char* stageId,
                                         const char* fname, int line);

VNVEXTERNC void _VnV_injectionPoint_end(const char* packageName, const char* id,
                                       const char* fname, int line);

VNVEXTERNC void _VnV_registerInjectionPoint(const char* packageName,
                                            const char* id,
                                            const char* parameters_str);

// if not using VnV, then replace VnV macros with whitespace
#else

#  define INJECTION_POINT(...)
#  define INJECTION_POINT_C(...)
#  define INJECTION_LOOP_BEGIN_C(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define INJECTION_FUNCTION_WRAPPER_C(...)
#  define INJECTION_FUNCTION_WRAPPER(...)
#  define Register_Injection_Point(...)

#endif

#endif  // INJECTIONPOINTINTERFACE_H
