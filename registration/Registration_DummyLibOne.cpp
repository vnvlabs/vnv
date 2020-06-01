/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME DummyLibOne
#include "VnV.h" 
const char* getFullRegistrationJson_DummyLibOne(){
	 return "{\"InjectionPoints\":{\"class2_function1\":{\"name\":\"class2_function1\",\"packageName\":\"DummyLibOne\",\"parameters\":[{\"x\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":7,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/libraries/dummyLibOne/src/class2.cpp\",\"lineColumn\":5,\"lineNumber\":64}}}}}}";}

INJECTION_REGISTRATION(){
	Register_Injection_Point(Hello_temp_sub,"{\"intersection\":\"double\",\"slope\":\"double\",\"value\":\"double\",\"x\":\"int\"}");
	Register_Injection_Point(class2_function1,"{\"x\":\"int\"}");
	REGISTER_FULL_JSON(getFullRegistrationJson_DummyLibOne);
};



