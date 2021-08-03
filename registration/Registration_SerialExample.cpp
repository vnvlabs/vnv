///10278250045686125962
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SerialExample

#include "VnV.h" 
DECLAREOPTIONS(SerialExample)
const char* getFullRegistrationJson_SerialExample(){
	 return "{\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"Introduction\":\"\\n Sample Executable\\n =================\\n\\n This executable is designed to show a number of different injection points.\\n The first stage is to initialize the library. The text that appears in the\\n comment above the initialize call represents the introduction in the final\\n report.\\n   \",\"Options\":\"\\n Options Documentation\\n =====================\\n\\n This is a quick blurb before the options command that will be used at some\\n point to aid in the development of input files.\\n \"}";}

INJECTION_REGISTRATION(SerialExample){
	REGISTEROPTIONS(SerialExample)
	REGISTER_FULL_JSON(SerialExample, getFullRegistrationJson_SerialExample);
};



