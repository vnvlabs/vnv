#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

//if not NOT using VnV
//  i.e., if using VnV
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

//inserts a call to _VnV_injectionPoint to begin an instance of an injection point
//  COMM implements the boost communicator (see Communication.h)
//    the user inputs a VnV macro
//  VNV_STR(PNAME) inserts a string literal of PNAME
//  VNV_STR(NAME) inserts a string literal of NAME
//  VNV_FUNCTION_SIG inserts a call to _VnV_function_sig(a,b) with ...
//    a as the compiler
//    b as a char* of the "prettified" version of the function name (where the function is inserted as b)
//  __FILE__ is a macro that inserts the name of the source file that calls the INJECTION_LOOP_BEGIN_C macro
//  __LINE__ is a macro that inserts the line number that calls INJECTION_LOOP_BEGIN_C in the relevant source file
//  callback sets the callback to write injection point data to engine
//  EVERYONE(__VA_ARGS__) iterates through all variadic args and ...
//    //TODO explain what EVERYONE(__VA_ARGS__) does
//  VNV_END_PARAMETERS_S inserts a call to VNV_STR(VNV_END_PARAMETERS), where VNV_END_PARAMETERS inserts a call to __vnv_end_parameters__
//    //TODO explain what __vnv_end_parameters__ does
#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...) \
    _VnV_injectionPoint(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                        EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

//special case of INJECTION_POINT_C where ...
//  callback=NULL
//  ...=__VA_ARGS__, where "__VA_ARGS__" represents the variadic arguments that the user inputs as an argument for the "..." parameter
#  define INJECTION_POINT(PNAME, COMM, NAME, ...) \
    INJECTION_POINT_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

//inserts a call to to _VnV_injectionPoint_begin to start an injection point "loop" instance
//  see documentation for INJECTION_POINT_C for info on parameters
#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...) \
    _VnV_injectionPoint_begin(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

//special case of INJECTION_LOOP_BEGIN_C where ...
//  callback=NULL
//  ...=__VA_ARGS__ 
#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, NULL, __VA_ARGS__)

//inserts a call to _VnV_injectionPoint_end to end an injection point "loop" instance
#  define INJECTION_LOOP_END(PNAME, NAME) \
    _VnV_injectionPoint_end(VNV_STR(PNAME), VNV_STR(NAME), __FILE__, __LINE__);

//inserts a call to _VnV_injectionPoint_loop which calls the injection point during an injection point loop to insert data for a stage
#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), VNV_STR(STAGE), __FILE__, __LINE__);

//FIXME remove ) after STAGE ?
#  define INJECTION_LOOP_ITER_D(PNAME, NAME, STAGE) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), STAGE), __FILE__, __LINE__);

// //TODO doc
#  define INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, callback, \
                                       ...)                                   \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, __VA_ARGS__);                   \
    function(__VA_ARGS__);                                                    \
    INJECTION_LOOP_END(PNAME, NAME)

//special case of INJECTION_FUNCTION_WRAPPER_C where...
//  callback=NULL
//  ...=__VA_ARGS__
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

//function declarations
//  definitions located in /vnv/injection/src/c-interfaces/points/Injection.cpp
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
