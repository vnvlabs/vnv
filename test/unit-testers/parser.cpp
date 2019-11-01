#ifndef _EuclideanError_H 
#define _EuclideanError_H 

#include "VnV-Interfaces.h"
#include "VnV.h"

using namespace VnV;

class ParserUnitTests : public IUnitTester { 
public:

    // The run function is called to run the unit tester. This function should 
    // contain all the injection points needed to test the specified code. The getInputJson
    // function must return a json object that contains the configuration for all injection 
    // points that are to be included in the testing routine. 
    //
    // The runtime object will build up and run all tests at each injection point as specified
    // in the getInputJson return. 
    //
    // After the tests have run, the verify result function will be called. That call will pass
    // a json object containing the output from each of the injection points, and the associated 
    // tests. The JsonEngine is used for that file.  The object is a json array containing 
    // injection point objects. Each injection point object contains the name, and an array 
    // of tests completed at that injection point. The tests themselves will have stages 
    // as defined. 
    //
    // The verifyResult function should return true or false to indicate is all tests ran
    // successfully. 
    //
    // The user is also given a IOutputEngine ptr to allow writing a testing report that can
    // be comsumed by the hugo report generation code. 
    void run() {

	int x = 10; int y = 11;
	INJECTION_POINT(-1, "parser-test-x-y", int, x, int, y);

        x += 12;
	INJECTION_POINT(-1, "parser-test-x-y-2", int, x, int, y);
    }

     std::string getInputJson() {
        return R"({"not-implemented-yet" : "see" })";
    }
    
    bool verifyResult(IOutputEngine *results) {
    	return true;    
    }	     
};



extern "C" { 
   IUnitTester* ParserUnitTests_maker(TestConfig &config) {
	return new ParserUnitTests(config);
   } 
};

class ParserUnitTests_proxy { 
public: 
	ParserUnitTests_proxy(){ 
        VnV_registerUnitTester("Parser",ParserUnitTester_maker); 
  }
};

ParserUnitTests_proxy parserUnitTests_proxy;

#endif
