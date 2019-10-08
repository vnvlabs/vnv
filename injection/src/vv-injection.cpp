
#include "vv-output.h"
#include "vv-testing.h"
#include "vv-injection.h"
#include <iostream>

using namespace VnV;

InjectionPoint::InjectionPoint(std::string scope ) : m_scope(scope) {
}

std::string InjectionPoint::getScope() const {
    return m_scope;
}


void InjectionPoint::addTest(TestConfig config) {
  
    ITest* test = TestStore::getTestStore().getTest(config);
  
    if ( test != NULL  ) {
      m_tests.push_back(test);
    } 
}

InjectionPoint::InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, plist &params ) :
    m_scope(scope)
{
}


bool InjectionPoint::hasTests() {
  return m_tests.size() > 0 ; 
}

void InjectionPoint::unpack_parameters(NTV &ntv, va_list argp) {

    while (1) {
       std::string variableType = va_arg(argp, char*);
       if ( variableType == "__VV_END_PARAMETERS__" ) {
            break;
       } 
       std::string variableName = va_arg(argp, char*);
       void* variablePtr = va_arg(argp,void*);
       ntv.insert(std::make_pair(variableName, std::make_pair(variableType, variablePtr)));
    }    
}

void InjectionPoint::runTests(int stageValue ,va_list argp ) {

    OutputEngineManager *wrapper = EngineStore::getEngineStore().getEngineManager();
    NTV ntv;
    unpack_parameters(ntv, argp);

    // Call the method to write this injection point to file.
    wrapper->startInjectionPoint(getScope(),stageValue);
    
    for ( auto it : m_tests ) {
        it->_runTest(wrapper->getOutputEngine(), stageValue,ntv);
    }
    wrapper->endInjectionPoint(getScope(),stageValue);
}

InjectionPointStore::InjectionPointStore() {}

InjectionPoint* InjectionPointStore::getInjectionPoint(std::string key){
    auto it = injectionPoints.find(key);
    if ( it != injectionPoints.end() ) {
      return it->second;
    }
    return NULL;
}

InjectionPointStore& InjectionPointStore::getInjectionPointStore() {
      static InjectionPointStore *store = new InjectionPointStore();
      return *store;      
}

void InjectionPointStore::addInjectionPoint(std::string key, InjectionPoint *point) {
   injectionPoints.insert(std::make_pair(key,point));
}


