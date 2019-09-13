#ifndef VV_dummyTest_H 
#define VV_dummyTest_H 

#include "vv-testing.h"

class dummyTest : public IVVTest { 
public:

	 TestStatus runTest(IVVOutputEngine *engine, int testStage, double* slope, double *intersection ) { 
        
        //Write the slope and the intersection point to the VV output file.  
        engine->Put("slope", *slope);
        engine->Put("intersection", *intersection); 
        return SUCCESS;   
   }

   static void DeclareIO(IVVOutputEngine *engine) {
     engine->DefineDouble("slope"); 
     engine->DefineDouble("intersection");      
   }
    

	 dummyTest(VVTestConfig config) : IVVTest(config) {
		 m_parameters.insert(std::make_pair("slope","double"));
		 m_parameters.insert(std::make_pair("intersection","double"));
	}

	TestStatus runTest(IVVOutputEngine *engine, int stage, NTV& parameters ) {    
    double* x = carefull_cast<double>(stage,"slope", parameters); 
		double* y = carefull_cast<double>(stage,"intersection", parameters); 
		int testStage = m_config.getStage(stage).testStageId;    
    return runTest(engine, testStage,x,y);
	}

};

extern "C" { 
	IVVTest* dummyTest_maker(VVTestConfig &config) {
		return new dummyTest(config);
	}
  
  void dummyTest_DeclareIO(IVVOutputEngine *engine) {
    dummyTest::DeclareIO(engine);
  }
  
};

class dummyTest_proxy { 
public: 
	dummyTest_proxy(){ 
    VV_registerTest("dummyTest",dummyTest_maker, dummyTest_DeclareIO); 
  }
};

dummyTest_proxy p;

#endif
