
#define START_INJECTION_POINT 1
#define CP_INJECTION_POINT 2
#define END_INJECTION_POINT 3
#define SINGLE_INJECTION_POINT 4
#define INJECTED_TEST 5
#define INTRODUCTION 6
#define CONCLUSION 7

#define VA_UNPACKER_0(...) 
#define VA_UNPACKER_1(first,...) , #first VA_UNPACKER_0(__VA_ARGS__)
#define VA_UNPACKER_2(first,...) , #first VA_UNPACKER_1(__VA_ARGS__)
#define VA_UNPACKER_3(first,...) , #first VA_UNPACKER_2(__VA_ARGS__)
#define VA_UNPACKER_4(first,...) , #first VA_UNPACKER_3(__VA_ARGS__)
#define VA_UNPACKER_5(first,...) , #first VA_UNPACKER_4(__VA_ARGS__)
#define VA_UNPACKER_6(first,...) , #first VA_UNPACKER_5(__VA_ARGS__)
#define VA_UNPACKER_7(first,...) , #first VA_UNPACKER_6(__VA_ARGS__)
#define VA_UNPACKER_8(first,...) , #first VA_UNPACKER_7(__VA_ARGS__)
#define VA_UNPACKER_9(first,...) , #first VA_UNPACKER_8(__VA_ARGS__)
#define VA_UNPACKER_10(first,...) , #first VA_UNPACKER_9(__VA_ARGS__)
#define VA_UNPACKER_11(first,...) , #first VA_UNPACKER_10(__VA_ARGS__)
#define VA_UNPACKER_12(first,...) , #first VA_UNPACKER_11(__VA_ARGS__)
#define VA_UNPACKER_13(first,...) , #first VA_UNPACKER_12(__VA_ARGS__)
#define VA_UNPACKER_14(first,...) , #first VA_UNPACKER_13(__VA_ARGS__)
#define VA_UNPACKER_15(first,...) , #first VA_UNPACKER_14(__VA_ARGS__)
#define VA_UNPACKER_16(first,...) , #first VA_UNPACKER_15(__VA_ARGS__)
#define VA_UNPACKER_17(first,...) , #first VA_UNPACKER_16(__VA_ARGS__)
#define VA_UNPACKER_18(first,...) , #first VA_UNPACKER_17(__VA_ARGS__)
#define VA_UNPACKER_19(first,...) , #first VA_UNPACKER_18(__VA_ARGS__)
#define VA_UNPACKER_20(first,...) , #first VA_UNPACKER_19(__VA_ARGS__)
#define VA_UNPACKER_21(first,...) , #first VA_UNPACKER_20(__VA_ARGS__)
#define VA_UNPACKER_22(first,...) , #first VA_UNPACKER_21(__VA_ARGS__)
#define VA_UNPACKER_23(first,...) , #first VA_UNPACKER_22(__VA_ARGS__)
#define VA_UNPACKER_24(first,...) , #first VA_UNPACKER_23(__VA_ARGS__)
#define COUNT_UNPACKER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, num,...) num VA_UNPACKER_ ## num 
#define VA_UNPACK(...) COUNT_UNPACKER(__VA_ARGS__,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)(__VA_ARGS__)

#define EVERY_SECOND0(...)
#define EVERY_SECOND1_(second,...), (void*)(&second)  
#define EVERY_SECOND1(first,...) EVERY_SECOND1_(__VA_ARGS__)
#define EVERY_SECOND2_(second,...) , (void*)(&second) EVERY_SECOND1(__VA_ARGS__)
#define EVERY_SECOND2(first,...) EVERY_SECOND2_(__VA_ARGS__)
#define EVERY_SECOND3_(second,...) , (void*)(&second) EVERY_SECOND2(__VA_ARGS__)
#define EVERY_SECOND3(first,...) EVERY_SECOND3_(__VA_ARGS__)
#define EVERY_SECOND4_(second,...) , (void*)(&second) EVERY_SECOND3(__VA_ARGS__)
#define EVERY_SECOND4(first,...) EVERY_SECOND4_(__VA_ARGS__)
#define EVERY_SECOND5_(second,...) ,(void*)( &second) EVERY_SECOND4(__VA_ARGS__)
#define EVERY_SECOND5(first,...) EVERY_SECOND5_(__VA_ARGS__)
#define EVERY_SECOND6_(second,...) , (void*)(&second) EVERY_SECOND5(__VA_ARGS__)
#define EVERY_SECOND6(first,...) EVERY_SECOND6_(__VA_ARGS__)
#define EVERY_SECOND7_(second,...) , (void*)(&second) EVERY_SECOND6(__VA_ARGS__)
#define EVERY_SECOND7(first,...) EVERY_SECOND7_(__VA_ARGS__)
#define EVERY_SECOND8_(second,...) , (void*)(&second) EVERY_SECOND7(__VA_ARGS__)
#define EVERY_SECOND8(first,...) EVERY_SECOND8_(__VA_ARGS__)
#define EVERY_SECOND9_(second,...) , (void*)(&second) EVERY_SECOND8(__VA_ARGS__)
#define EVERY_SECOND9(first,...) EVERY_SECOND9_(__VA_ARGS__)
#define EVERY_SECOND10_(second,...) , (void*)(&second) EVERY_SECOND9(__VA_ARGS__)
#define EVERY_SECOND10(first,...) EVERY_SECOND10_(__VA_ARGS__)
#define EVERY_SECOND11_(second,...) , (void*)(&second) EVERY_SECOND10(__VA_ARGS__)
#define EVERY_SECOND11(first,...) EVERY_SECOND11_(__VA_ARGS__)
#define EVERY_SECOND12_(second,...) , (void*)(&second) EVERY_SECOND11(__VA_ARGS__)
#define EVERY_SECOND12(first,...) EVERY_SECOND12_(__VA_ARGS__)
#define EVERY_SECOND13_(second,...) , (void*)(&second) EVERY_SECOND12(__VA_ARGS__)
#define EVERY_SECOND13(first,...) EVERY_SECOND13_(__VA_ARGS__)
#define EVERY_SECOND14_(second,...) , (void*)(&second) EVERY_SECOND13(__VA_ARGS__)
#define EVERY_SECOND14(first,...) EVERY_SECOND14_(__VA_ARGS__)
#define EVERY_SECOND15_(second,...) , (void*)(&second) EVERY_SECOND14(__VA_ARGS__)
#define EVERY_SECOND15(first,...) EVERY_SECOND15_(__VA_ARGS__)
#define EVERY_SECOND16_(second,...) , (void*)(&second) EVERY_SECOND15(__VA_ARGS__)
#define EVERY_SECOND16(first,...) EVERY_SECOND16_(__VA_ARGS__)
#define EVERY_SECOND17_(second,...) , (void*)(&second) EVERY_SECOND16(__VA_ARGS__)
#define EVERY_SECOND17(first,...) EVERY_SECOND17_(__VA_ARGS__)
#define EVERY_SECOND18_(second,...) , (void*)(&second) EVERY_SECOND17(__VA_ARGS__)
#define EVERY_SECOND18(first,...) EVERY_SECOND18_(__VA_ARGS__)
#define EVERY_SECOND19_(second,...) , (void*)(&second) EVERY_SECOND18(__VA_ARGS__)
#define EVERY_SECOND19(first,...) EVERY_SECOND19_(__VA_ARGS__)
#define EVERY_SECOND20_(second,...) , (void*)(&second) EVERY_SECOND19(__VA_ARGS__)
#define EVERY_SECOND20(first,...) EVERY_SECOND20_(__VA_ARGS__)
#define EVERY_SECOND21_(second,...) , (void*)(&second) EVERY_SECOND20(__VA_ARGS__)
#define EVERY_SECOND21(first,...) EVERY_SECOND21_(__VA_ARGS__)
#define EVERY_SECOND22_(second,...) , (void*)(&second) EVERY_SECOND21(__VA_ARGS__)
#define EVERY_SECOND22(first,...) EVERY_SECOND22_(__VA_ARGS__)
#define EVERY_SECOND23_(second,...) , (void*)(&second) EVERY_SECOND22(__VA_ARGS__)
#define EVERY_SECOND23(first,...) EVERY_SECOND23_(__VA_ARGS__)
#define EVERY_SECOND24_(second,...) , (void*)(&second) EVERY_SECOND23(__VA_ARGS__)
#define EVERY_SECOND24(first,...) EVERY_SECOND24_(__VA_ARGS__)
#define COUNT_EVERY_SECOND(_1,__1,_2,__2,_3,__3,_4,__4,_5,__5,_6,__6,_7,__7,_8,__8,_9,__9,_10,__10,_11,__11,_12,__12,_13,__13,_14,__14,_15,__15,_16,__16,_17,__17,_18,__18,_19,__19,_20,__20,_21,__21,_22,__22,_23,__23,_24,__24,num,...) EVERY_SECOND ## num 
#define EVERY_SECOND(...) COUNT_EVERY_SECOND(__VA_ARGS__,24,24,23,23,22,22,21,21,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0)(__VA_ARGS__)

#define INJECTION_POINT(NAME,STAGE) VV_injectionPoint(STAGE,#NAME,__FUNCTION__ EVERY_SECOND( NAME##_VVTest));

#define REGISTER_IP(NAME,STAGE,DESC) static int NAME ## STAGE = VV_registration(#NAME, STAGE , __FILE__,__LINE__,DESC, VA_UNPACK(NAME ## _VVTest) );

#ifdef __cplusplus
 #define EXTERNC extern "C" 

#else
 #define EXTERNC 
#endif

EXTERNC void VV_injectionPoint(int stageVal, const char * id, const char * function, ...); 
EXTERNC int  VV_registration(const char * scope, int stage, const char * filename, int line, const char * desc, int count, ...);
EXTERNC int VV_writeXML(const char *filename);
EXTERNC int VV_init(const char *filename);
EXTERNC int VV_finialze();


