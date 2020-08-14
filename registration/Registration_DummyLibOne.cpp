///10060559065298721827
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: DummyLibOne

#include "VnV.h" 
const char* getFullRegistrationJson_DummyLibOne(){
	 return "{\"InjectionPoints\":{\"class2_function1\":{\"docs\":\"\",\"name\":\"class2_function1\",\"packageName\":\"DummyLibOne\",\"parameters\":[{\"x\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":7,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/libraries/dummyLibOne/src/class2.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}},\"function_in_header\":{\"docs\":\"\",\"name\":\"function_in_header\",\"packageName\":\"DummyLibOne\",\"parameters\":[{\"y\":\"int\"},{\"y\":\"int\"},{\"y\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function2\",\"Calling Function Column\":3,\"Calling Function Line\":12,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/libraries/dummyLibOne/include/dlclass1.h\",\"lineColumn\":5,\"lineNumber\":79}}}}}}";}

INJECTION_REGISTRATION(DummyLibOne){
	Register_Injection_Point(DummyLibOne,Hello_temp_sub,"{\"intersection\":\"double\",\"slope\":\"double\",\"value\":\"double\",\"x\":\"int\"}");
	Register_Injection_Point(DummyLibOne,class2_function1,"{\"x\":\"int\"}");
	Register_Injection_Point(DummyLibOne,function_in_header,"{\"y\":\"int\"}");
	REGISTER_FULL_JSON(DummyLibOne, getFullRegistrationJson_DummyLibOne);
};



