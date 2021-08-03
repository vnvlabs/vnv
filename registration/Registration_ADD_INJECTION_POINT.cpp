///14685863980323803460
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: ADD_INJECTION_POINT

#include "VnV.h" 
const char* getFullRegistrationJson_ADD_INJECTION_POINT(){
	 return "{}";}

INJECTION_REGISTRATION(ADD_INJECTION_POINT){
	Register_Injection_Point("ADD_INJECTION_POINT","MY_FIRST_INJECTION_POINT","{\"argc\":\"int\"}");
	REGISTER_FULL_JSON(ADD_INJECTION_POINT, getFullRegistrationJson_ADD_INJECTION_POINT);
};



