///5911978079149097461
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Newton

#include "VnV.h" 
const char* getFullRegistrationJson_Newton(){
	 return "{}";}

INJECTION_REGISTRATION(Newton){
	REGISTER_FULL_JSON(Newton, getFullRegistrationJson_Newton);
};



