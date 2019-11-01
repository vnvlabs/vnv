

#ifndef VV_VNV_H
#define VV_VNV_H

#include <stdarg.h>

#include <string>

#define START_INJECTION_POINT 1
#define CP_INJECTION_POINT 2
#define END_INJECTION_POINT 3
#define SINGLE_INJECTION_POINT 4
#define INJECTED_TEST 5
#define INTRODUCTION 6
#define CONCLUSION 7

#define EVERY_SECOND0(...)
#define EVERY_SECOND1_(second, ...)
#define EVERY_SECOND1(first, ...)
#define EVERY_SECOND2_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND1(__VA_ARGS__)
#define EVERY_SECOND2(first, ...) #first, EVERY_SECOND2_(__VA_ARGS__)
#define EVERY_SECOND3_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND2(__VA_ARGS__)
#define EVERY_SECOND3(first, ...) #first, EVERY_SECOND3_(__VA_ARGS__)
#define EVERY_SECOND4_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND3(__VA_ARGS__)
#define EVERY_SECOND4(first, ...) #first, EVERY_SECOND4_(__VA_ARGS__)
#define EVERY_SECOND5_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND4(__VA_ARGS__)
#define EVERY_SECOND5(first, ...) #first, EVERY_SECOND5_(__VA_ARGS__)
#define EVERY_SECOND6_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND5(__VA_ARGS__)
#define EVERY_SECOND6(first, ...) #first, EVERY_SECOND6_(__VA_ARGS__)
#define EVERY_SECOND7_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND6(__VA_ARGS__)
#define EVERY_SECOND7(first, ...) #first, EVERY_SECOND7_(__VA_ARGS__)
#define EVERY_SECOND8_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND7(__VA_ARGS__)
#define EVERY_SECOND8(first, ...) #first, EVERY_SECOND8_(__VA_ARGS__)
#define EVERY_SECOND9_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND8(__VA_ARGS__)
#define EVERY_SECOND9(first, ...) #first, EVERY_SECOND9_(__VA_ARGS__)
#define EVERY_SECOND10_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND9(__VA_ARGS__)
#define EVERY_SECOND10(first, ...) #first, EVERY_SECOND10_(__VA_ARGS__)
#define EVERY_SECOND11_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND10(__VA_ARGS__)
#define EVERY_SECOND11(first, ...) #first, EVERY_SECOND11_(__VA_ARGS__)
#define EVERY_SECOND12_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND11(__VA_ARGS__)
#define EVERY_SECOND12(first, ...) #first, EVERY_SECOND12_(__VA_ARGS__)
#define EVERY_SECOND13_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND12(__VA_ARGS__)
#define EVERY_SECOND13(first, ...) #first, EVERY_SECOND13_(__VA_ARGS__)
#define EVERY_SECOND14_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND13(__VA_ARGS__)
#define EVERY_SECOND14(first, ...) #first, EVERY_SECOND14_(__VA_ARGS__)
#define EVERY_SECOND15_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND14(__VA_ARGS__)
#define EVERY_SECOND15(first, ...) #first, EVERY_SECOND15_(__VA_ARGS__)
#define EVERY_SECOND16_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND15(__VA_ARGS__)
#define EVERY_SECOND16(first, ...) #first, EVERY_SECOND16_(__VA_ARGS__)
#define EVERY_SECOND17_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND16(__VA_ARGS__)
#define EVERY_SECOND17(first, ...) #first, EVERY_SECOND17_(__VA_ARGS__)
#define EVERY_SECOND18_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND17(__VA_ARGS__)
#define EVERY_SECOND18(first, ...) #first, EVERY_SECOND18_(__VA_ARGS__)
#define EVERY_SECOND19_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND18(__VA_ARGS__)
#define EVERY_SECOND19(first, ...) #first, EVERY_SECOND19_(__VA_ARGS__)
#define EVERY_SECOND20_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND19(__VA_ARGS__)
#define EVERY_SECOND20(first, ...) #first, EVERY_SECOND20_(__VA_ARGS__)
#define EVERY_SECOND21_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND20(__VA_ARGS__)
#define EVERY_SECOND21(first, ...) #first, EVERY_SECOND21_(__VA_ARGS__)
#define EVERY_SECOND22_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND21(__VA_ARGS__)
#define EVERY_SECOND22(first, ...) #first, EVERY_SECOND22_(__VA_ARGS__)
#define EVERY_SECOND23_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND22(__VA_ARGS__)
#define EVERY_SECOND23(first, ...) #first, EVERY_SECOND23_(__VA_ARGS__)
#define EVERY_SECOND24_(second, ...) \
#  second, (void*)(&second), EVERY_SECOND23(__VA_ARGS__)
#define EVERY_SECOND24(first, ...) , #first EVERY_SECOND24_(__VA_ARGS__)
#define COUNT_EVERY_SECOND(                                                    \
    _1, __1, _2, __2, _3, __3, _4, __4, _5, __5, _6, __6, _7, __7, _8, __8,    \
    _9, __9, _10, __10, _11, __11, _12, __12, _13, __13, _14, __14, _15, __15, \
    _16, __16, _17, __17, _18, __18, _19, __19, _20, __20, _21, __21, _22,     \
    __22, _23, __23, _24, __24, num, ...)                                      \
  EVERY_SECOND##num

#define EVERY_SECOND(...)                                                     \
  COUNT_EVERY_SECOND(__VA_ARGS__, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, \
                     19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,  \
                     12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, \
                     4, 3, 3, 2, 2, 1, 1, 0)                                  \
  (__VA_ARGS__)

// TODO Make this a compilter flag
#define WITH_VNV

#ifdef WITH_VNV

#  define INJECTION_POINT(NAME, STAGE, ...)        \
    VnV_injectionPoint(STAGE, #NAME, __FUNCTION__, \
                       EVERY_SECOND(__VA_ARGS__) "__VV_END_PARAMETERS__");

#  ifdef __cplusplus
#    define EXTERNC extern "C"
#  else
#    define EXTERNC
#  endif

EXTERNC void VnV_injectionPoint(int stageVal, const char* id,
                                const char* function, ...);
EXTERNC int VnV_init(int* argc, char*** argv, const char* filename);
EXTERNC int VnV_finalize();
EXTERNC int VnV_runUnitTests();
#  undef EXTERNC
#else
#  define INJECTION_POINT(NAME, STAGE, ...)
#  define VnV_injectionPoint(...)
#  define VnV_init(...)
#  define VnV_finalize() ;
#  define VnV_runUnitTests() ;
#endif

#endif
