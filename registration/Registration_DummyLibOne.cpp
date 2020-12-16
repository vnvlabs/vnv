///10467563174676490593
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: DummyLibOne

#include "VnV.h" 
const char* getFullRegistrationJson_DummyLibOne(){
	 return "{\"InjectionPoints\":{\"class2_function1\":{\"docs\":\"\",\"name\":\"class2_function1\",\"packageName\":\"DummyLibOne\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"}}},\"function_in_header\":{\"docs\":\"\",\"name\":\"function_in_header\",\"packageName\":\"DummyLibOne\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"}}}}}";}

INJECTION_REGISTRATION(DummyLibOne){
	Register_Injection_Point(DummyLibOne,Hello_temp_sub,0,"null");
	Register_Injection_Point(DummyLibOne,class2_function1,0,"null");
	Register_Injection_Point(DummyLibOne,function_in_header,0,"null");
	REGISTER_FULL_JSON(DummyLibOne, getFullRegistrationJson_DummyLibOne);
};



