/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME TestRunner
#include "VnV.h" 
DECLAREUNITTEST(BFSTester)
DECLAREUNITTEST(Demo)
DECLAREUNITTEST(bitvector)
DECLAREUNITTEST(router)
const char* getFullRegistrationJson_TestRunner(){
	 return "{\"UnitTests\":{\"BFSTester\":{\"docs\":\"\"},\"Demo\":{\"docs\":\"\"},\"bitvector\":{\"docs\":\"\n Unit tests for the sparse bit vector code.\n \"},\"router\":{\"docs\":\"\n Unit Tests for the Router Class of the Parallel Output Engine.\n .. vnv-results-table:: $.results\n \"}}}";}

INJECTION_REGISTRATION(){
	REGISTERUNITTEST(BFSTester);
	REGISTERUNITTEST(Demo);
	REGISTERUNITTEST(bitvector);
	REGISTERUNITTEST(router);
	REGISTER_FULL_JSON(getFullRegistrationJson_TestRunner);
};



