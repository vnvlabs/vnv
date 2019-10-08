#ifndef _EuclideanError_H 
#define _EuclideanError_H 

#include "vv-testing.h"
#include <math.h>
#include <iostream>
using namespace VnV;

class EuclideanError : public ITest { 
public:

	 TestStatus runTest(IOutputEngine *engine, int testStage, std::vector<double>* measured, std::vector<double> *exact ) { 
        
     
        if ( measured->size() != exact->size() )
        {  
            double m1 = -1;
            engine->Put("l2_error",m1 ) ;
            engine->Put("l1_error", m1 ) ;
            engine->Put("linf_error", m1 ) ;
            return FAILURE;
        }

        double l2(0), l1(0), linf(0);
        for ( int i = 0 ; i < measured->size(); i++ ) {
            double diff = fabs((*measured)[i] - (*exact)[i]);
            l1 += diff;
            l2 += diff*diff;    
            linf = ( diff > linf) ? diff : linf;
        } 
     
        l2 = sqrt(l2);
        engine->Put("l2_error", l2);
        engine->Put("l1_error", l1);
        engine->Put("linf_error",linf);
        return SUCCESS;   
   }

   static void DeclareIO(IOutputEngine *engine) {
     engine->DefineDouble("l2_error"); 
     engine->DefineDouble("l1_error");      
     engine->DefineDouble("linf_error");      
   }
    

	 EuclideanError(TestConfig config) : ITest(config) {
		 m_parameters.insert(std::make_pair("measured","std::vector<double>"));
		 m_parameters.insert(std::make_pair("exact","std::vector<double>"));
	}

	TestStatus runTest(IOutputEngine *engine, int stage, NTV& parameters ) {    
    std::vector<double>* x = carefull_cast<std::vector<double>>(stage,"measured", parameters); 
    std::vector<double>* y = carefull_cast<std::vector<double>>(stage,"exact", parameters); 
		int testStage = m_config.getStage(stage).getTestStageId();    
    return runTest(engine, testStage,x,y);
	}

};

extern "C" { 
	ITest* EuclideanError_maker(TestConfig &config) {
		return new EuclideanError(config);
	}
  
  void EuclideanError_DeclareIO(IOutputEngine *engine) {
    EuclideanError::DeclareIO(engine);
  }
  
};

class EuclideanError_proxy { 
public: 
	EuclideanError_proxy(){ 
   VnV_registerTest("EuclideanError",EuclideanError_maker, EuclideanError_DeclareIO); 
  }
};

EuclideanError_proxy euclideanError_proxy;

#endif
