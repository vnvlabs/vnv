///4208172715647325160
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SampleCExecutable

#include "VnV.h" 
DECLAREOPTIONS(SampleCExecutable)
const char* getFullRegistrationJson_SampleCExecutable(){
	 return "{\"Conclusion\":\"\",\"InjectionPoints\":{\"CFunction\":{\"docs\":\"\",\"name\":\"CFunction\",\"packageName\":\"SampleCExecutable\",\"parameters\":[{\"x\":\"int*\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":5,\"lineNumber\":21}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":43,\"lineNumber\":25}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/dummy/c_executables/sample.c\",\"lineColumn\":51,\"lineNumber\":27}}}}},\"Introduction\":\"\",\"Options\":\"\"}";}

INJECTION_REGISTRATION(SampleCExecutable){
	REGISTEROPTIONS(SampleCExecutable)
	Register_Injection_Point("SampleCExecutable","CFunction","{\"x\":\"int*\"}");
	REGISTER_FULL_JSON(SampleCExecutable, getFullRegistrationJson_SampleCExecutable);
};



