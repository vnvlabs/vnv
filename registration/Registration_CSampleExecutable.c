/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME CSampleExecutable
#include "VnV.h" 
DECLAREOPTIONS
const char* getFullRegistrationJson_CSampleExecutable(){
	 return "{\"InjectionPoints\":{\"CFunction\":{\"name\":\"CFunction\",\"packageName\":\"CSampleExecutable\",\"parameters\":[{\"x\":\"int*\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":8,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":5,\"lineNumber\":43}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":8,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":5,\"lineNumber\":53}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":8,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":5,\"lineNumber\":65}}}}},\"Options\":{\"docs\":\"\"}}";}

INJECTION_REGISTRATION(){
	REGISTEROPTIONS
	Register_Injection_Point(CFunction,"{\"x\":\"int*\"}");
	REGISTER_FULL_JSON(getFullRegistrationJson_CSampleExecutable);
};



