///15188705483593590288
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Dakota

#include "VnV.h" 
const char* getFullRegistrationJson_Dakota(){
	 return "{}";}

INJECTION_REGISTRATION(Dakota){
	REGISTER_FULL_JSON(Dakota, getFullRegistrationJson_Dakota);
};



