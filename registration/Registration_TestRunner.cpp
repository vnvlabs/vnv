/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME TestRunner
#include "VnV.h" 
DECLAREUNITTEST(AnotherDemo)
DECLAREUNITTEST(BFSTester)
DECLAREUNITTEST(Communication)
DECLAREUNITTEST(Demo)
DECLAREUNITTEST(bitvector)
DECLAREUNITTEST(router)
const char* getFullRegistrationJson_TestRunner(){
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"UnitTests\":{\"AnotherDemo\":{\"docs\":\"\"},\"BFSTester\":{\"docs\":\"\"},\"Communication\":{\"docs\":\"\"},\"Demo\":{\"docs\":\"\"},\"bitvector\":{\"docs\":\"\\n Unit tests for the sparse bit vector code.\\n \"},\"router\":{\"docs\":\"\\n Unit Tests for the Router Class of the Parallel Output Engine.\\n .. vnv-results-table:: $.results\\n \"}}}";}

INJECTION_REGISTRATION(){
	REGISTERUNITTEST(AnotherDemo);
	REGISTERUNITTEST(BFSTester);
	REGISTERUNITTEST(Communication);
	REGISTERUNITTEST(Demo);
	REGISTERUNITTEST(bitvector);
	REGISTERUNITTEST(router);
	VnV_Declare_Communicator("TestRunner","VNV","mpi");
	REGISTER_FULL_JSON(getFullRegistrationJson_TestRunner);
};



