///6014021851316554942
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: DummyTestLibOne

#include "VnV.h" 
DECLARETEST(DummyTestLibOne,EuclideanError)
DECLARETEST(DummyTestLibOne,dummyTest)
const char* getFullRegistrationJson_DummyTestLibOne(){
	 return "{\"Tests\":{\"EuclideanError\":{\"docs\":\"\\n  Euclidean Error. Docuementation is Fun\\n\\n  This is some more.\\n\\n\\n  This is the rest.\",\"parameters\":{\"exact_test\":\"(std::vector<double>)\",\"measured\":\"std::vector<double>\"}},\"dummyTest\":{\"docs\":\"\",\"parameters\":{\"intersection\":\"double\",\"slope\":\"double\"}}}}";}

INJECTION_REGISTRATION(DummyTestLibOne){
	REGISTERTEST(DummyTestLibOne,EuclideanError);
	REGISTERTEST(DummyTestLibOne,dummyTest);
	REGISTER_FULL_JSON(DummyTestLibOne, getFullRegistrationJson_DummyTestLibOne);
};



