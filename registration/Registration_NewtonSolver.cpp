/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME NewtonSolver
#include "VnV.h" 
const char* getFullRegistrationJson_NewtonSolver(){
	 return "{}";}

INJECTION_REGISTRATION(){
	REGISTER_FULL_JSON(getFullRegistrationJson_NewtonSolver);
};



