///14257744734146834390
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Dakota

#include "VnV.h" 
DECLAREITERATOR(Dakota,parameter_search)
const char* getFullRegistrationJson_Dakota(){
	 return "{\"Iterators\":{\"parameter_search\":{\"docs\":\"\\n @brief INJECTION_ITERATOR_R\\n @param x0\\n @param x1\\n @param f\\n\\n This one needs some work. All this code could be made more generic so we can support a bunch\\n of different tests with little changes. It also doesn\'t work because inputfile is hardcoded\\n string to a file that does not exist. WIP.\\n\\n The goal here is to create a set of dakota tests for a lot of different methods. We want to make\\n it really easy to write a dakota test. We want to hide the fact that dakota is doing all the work\\n\\n So a test called parameter search that accepts x, y, z, w, ... as parameters as allows the user to\\n run a parameter search.\\n\\n A test that does UQ on parameters\\n etc.\\n\\n Basically, we wrap dakota up so it looks like VnV then let dakota drive.\\n\\n \",\"parameters\":{\"f\":\"double\",\"x0\":\"double\",\"x1\":\"double\"}}}}";}

INJECTION_REGISTRATION(Dakota){
	REGISTERITERATOR(Dakota,parameter_search);
	REGISTER_FULL_JSON(Dakota, getFullRegistrationJson_Dakota);
};



