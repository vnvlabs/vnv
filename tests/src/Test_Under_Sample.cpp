#ifndef VV_Test_Under_Sample_H 
#define VV_Test_Under_Sample_H 

#include "injection.h"


class Test_Under_Sample : public IVVTest { 
public:

	 TestStatus runTest(int testStage, int* x,int* y,int* z,int* w) { 
        
        // This is the main function where the test is executed. Tests are assigned to 
        // injection points using the configuration xml file. That configuration file 
        // specifies the modifiers required to get the correct transforms. 
        //
        // By the time we have reached this point -- all the mappings have been done and we 
        // have the four variables as specified by the injection point config and the test.
        //
         
        

        std::cout << " We made into this test at stage " << testStage << " " << *x << " " << *y << " " << *z << " " << *w << std::endl;
        return SUCCESS;   
   }

   static void DeclareIO(adios2::IO &io) {
     std::cout << " Made it into the decalre IO function for the test " << std::endl;
      
     // Here we should declare the varialbes used in the test. This function is called 
     // once when the test is first associated with an injection point. The idea here 
     // is to describe the variables to adios. You will then set the varaibles during
     // the run test routine. 
     //
     // Note: For now, in the dev version. You can declare a variable at any point in time.
     // Once this thing is done, we can look at optimizing IO by locking the adios definitions
     // prior to beginning tests. 
     
   }
    
	 Test_Under_Sample(VVTestConfig config) : IVVTest(config) {
		 m_parameters.insert(std::make_pair("x","int"));
		 m_parameters.insert(std::make_pair("y","int"));
		 m_parameters.insert(std::make_pair("z","int"));
		 m_parameters.insert(std::make_pair("w","int"));
	}

	TestStatus runTest(int stage, NTV& parameters ) {    
    int* x = carefull_cast<int>(stage,"x", parameters); 
		int* y = carefull_cast<int>(stage,"y", parameters); 
		int* z = carefull_cast<int>(stage,"z", parameters); 
		int* w = carefull_cast<int>(stage,"w", parameters); 
		int testStage = m_config.getStage(stage).testStageId;    
      
      return runTest(testStage,x,y,z,w);
	}

};
/*
TEST_MODIFIER(NAME, VALUE) {
  if ( ip.first.compare("KSP") && tp.compare("PC") ) {
      KSP* ksp = (KSP*) ip.second;
      PC pc;
      KSPGetPC(*ksp,&pc);
      return (void*) pc;
  }
  throw "This modifier does not apply";
}
REGISTER_TEST_MODIFER(UID,NAME)
*/
////// Modifiers -- Injection points are not always going to be in the 
// form that we need them in general tests -- so, we need to "modify" 
// them when they come into the tests. 
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
