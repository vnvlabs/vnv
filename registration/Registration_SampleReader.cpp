///2623259724921982557
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SampleReader

#include "VnV.h" 
const char* getFullRegistrationJson_SampleReader(){
	 return "{}";}

INJECTION_REGISTRATION(SampleReader){
	REGISTER_FULL_JSON(SampleReader, getFullRegistrationJson_SampleReader);
};



