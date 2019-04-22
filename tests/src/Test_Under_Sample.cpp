#ifndef VV_Test_Under_Sample_H 
#define VV_Test_Under_Sample_H 

#include "injection.h"

class Test_Under_Sample : public IVVTest { 
public:

	 TestStatus runTest(adios2::Engine &engine, int testStage, double* slope, double *intersection ) { 
        
        //Write the slope and the intersection point to the VV output file.  
        engine.Put("slope", *slope);
        engine.Put("intersection", *intersection); 
        std::cout << "The slope is " << *slope << " and the intersection is " << *intersection << std::endl;
        return SUCCESS;   
   }

   static void DeclareIO(adios2::IO &io) {
     io.DefineVariable<double>("slope"); 
     io.DefineVariable<double>("intersection");      
   }
    

	 Test_Under_Sample(VVTestConfig config) : IVVTest(config) {
		 m_parameters.insert(std::make_pair("slope","double"));
		 m_parameters.insert(std::make_pair("intersection","double"));
	}

	TestStatus runTest(adios2::Engine &engine, int stage, NTV& parameters ) {    
    double* x = carefull_cast<double>(stage,"slope", parameters); 
		double* y = carefull_cast<double>(stage,"intersection", parameters); 
		int testStage = m_config.getStage(stage).testStageId;    
    return runTest(engine, testStage,x,y);
	}

};

class TestModifier : public IVVTransform {
  
  void * Transform(std::pair<std::string, void*> ip, std::string tp) {
      std::cout << " We are in the test modifier defined in the tests library " << std::endl;
      if ( ip.first.compare(tp) == 0 ) {
        return ip.second;
      }
      throw " This sample modifier does the same thing as the default, and it failed " ; 
  }

};

extern "C" { 
	IVVTest* Test_Under_Sample_maker(VVTestConfig &config) {
		return new Test_Under_Sample(config);
	}
  
  void Test_Under_Sample_DeclareIO(adios2::IO& io) {
    Test_Under_Sample::DeclareIO(io);
  }
  
  IVVTransform* Test_Under_Sample_modifier() {
    return new TestModifier();
  }
};

class Test_Under_Sample_proxy { 
public: 
	Test_Under_Sample_proxy(){ 
     
    // Register the test with the factory 
    VV::test_factory["Test_Under_Sample"] = std::make_pair(Test_Under_Sample_maker, Test_Under_Sample_DeclareIO); 
    VV::trans_factory["TestModifier"] = Test_Under_Sample_modifier;  
  }
};

Test_Under_Sample_proxy p;








#endif
