///5019719238243340362
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SimpleExecutable

#include "VnV.h" 
const char* getFullRegistrationJson_SimpleExecutable(){
	 return "{}";}

INJECTION_REGISTRATION(SimpleExecutable){
	Register_Injection_Iterator("SimpleExecutable","FunctionEvaluation","{\"f\":\"double\",\"x\":\"double\"}");
	REGISTER_FULL_JSON(SimpleExecutable, getFullRegistrationJson_SimpleExecutable);
};



