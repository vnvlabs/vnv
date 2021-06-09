#ifndef PACKAGENAME_H
#define PACKAGENAME_H

#include <stdarg.h>

#define VNVPACKAGENAME VNV
#define VNVPACKAGENAME_S "VNV"

#define VnV_E_STR(x) #x
#define VNV_STR(x) VnV_E_STR(x)
#define VNV_EX(x) x
#define VNV_JOIN(x,y,z) x##y##z

#define VNVREGNAME __vnv_registration_callback__

#if __cplusplus
#  define VNVEXTERNC extern "C"
#else
#  define VNVEXTERNC
#endif

// C Injection Macros.

#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X) FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X) FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X) FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X) FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X) FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X) FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X) FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...) WHAT(X) FE_8(WHAT, __VA_ARGS__)
#define FE_10(WHAT, X, ...) WHAT(X) FE_9(WHAT, __VA_ARGS__)
#define FE_11(WHAT, X, ...) WHAT(X) FE_10(WHAT, __VA_ARGS__)
#define FE_12(WHAT, X, ...) WHAT(X) FE_11(WHAT, __VA_ARGS__)
#define FE_13(WHAT, X, ...) WHAT(X) FE_12(WHAT, __VA_ARGS__)
#define FE_14(WHAT, X, ...) WHAT(X) FE_13(WHAT, __VA_ARGS__)
#define FE_15(WHAT, X, ...) WHAT(X) FE_14(WHAT, __VA_ARGS__)
#define FE_16(WHAT, X, ...) WHAT(X) FE_15(WHAT, __VA_ARGS__)
#define FE_17(WHAT, X, ...) WHAT(X) FE_16(WHAT, __VA_ARGS__)
#define FE_18(WHAT, X, ...) WHAT(X) FE_17(WHAT, __VA_ARGS__)
#define FE_19(WHAT, X, ...) WHAT(X) FE_18(WHAT, __VA_ARGS__)
#define FE_20(WHAT, X, ...) WHAT(X) FE_19(WHAT, __VA_ARGS__)
#define FE_21(WHAT, X, ...) WHAT(X) FE_20(WHAT, __VA_ARGS__)
#define FE_22(WHAT, X, ...) WHAT(X) FE_21(WHAT, __VA_ARGS__)
#define FE_23(WHAT, X, ...) WHAT(X) FE_22(WHAT, __VA_ARGS__)
#define FE_24(WHAT, X, ...) WHAT(X) FE_23(WHAT, __VA_ARGS__)
#define FE_25(WHAT, X, ...) WHAT(X) FE_24(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, \
                  _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, \
                  NAME, ...)                                                  \
  NAME

#define FOR_EACH(action, ...)                                                 \
  GET_MACRO(_0, __VA_ARGS__, FE_25, FE_24, FE_23, FE_22, FE_21, FE_20, FE_19, \
            FE_18, FE_17, FE_16, FE_15, FE_14, FE_13, FE_12, FE_11, FE_10,    \
            FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1, FE_0)       \
  (action, __VA_ARGS__)

#define VNV_END_PARAMETERS __vnv_end_parameters__
#define VNV_END_PARAMETERS_S VNV_STR(VNV_END_PARAMETERS)

#define REG_HELPER_(X, Y) X##Y
#define REG_HELPER(X, Y) REG_HELPER_(X, Y)

#define VNV_REGISTRATION_CALLBACK_NAME(PNAME) REG_HELPER(VNVREGNAME, PNAME)
#define VNV_GET_REGISTRATION "__vnv_registration_callback__"

#ifdef __cplusplus
#  define INJECTION_REGISTRATION(PNAME) \
    extern "C" void __attribute__((visibility("default"))) VNV_REGISTRATION_CALLBACK_NAME(PNAME)()
#else
#  define INJECTION_REGISTRATION(PNAME) \
    void __attribute__((visibility("default"))) VNV_REGISTRATION_CALLBACK_NAME(PNAME)()
#endif

#define INJECTION_REGISTRATION_CALL(PNAME) VNV_REGISTRATION_CALLBACK_NAME(PNAME)
#define INJECTION_REGISTRATION_PTR(PNAME) &VNV_REGISTRATION_CALLBACK_NAME(PNAME)

// This macro allows the user to enter some documentation about the package to
// be picked up by the clang parser.
#define INJECTION_PACKAGEDOCS(PNAME)

// Forward declare the VNV Registration Function.
INJECTION_REGISTRATION(VNVPACKAGENAME);

#define VNV_INCLUDED

#endif  // PACKAGENAME_H
