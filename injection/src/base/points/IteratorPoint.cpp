
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined
 * in InjectionPoint.h **/

#include "base/points/InjectionPoint.h"

#include <iostream>

#include "base/stores/InjectionPointStore.h"
#include "base/Logger.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/TestStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
#include "interfaces/IIterator.h"
#include "base/stores/TestStore.h"
#include "base/stores/IteratorStore.h"
#include "base/stores/IteratorsStore.h"

using namespace VnV;

using nlohmann::json_schema::json_validator;



void IterationPoint::addIterator(IteratorConfig &config) {
  config.setParameterMap(parameterMap);
  std::shared_ptr<IIterator> test = IteratorsStore::instance().getTest(config);

  if (test != nullptr) {
    m_iterators.push_back(test);
    return;
  }

  VnV_Error(VNVPACKAGENAME, "Error Loading Iterator with Name %s", config.getName().c_str());
}


bool IterationPoint::iterate() {
    
    OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();
    
    // On the first iteration, call INjectionPointBegin and run all the tests. 
    if (started == 0) {
      InjectionPointBase::setInjectionPointType(InjectionPointType::Begin, "Begin");
      InjectionPoint::run();
      started++;
      return true;
    } else {
      InjectionPointBase::setInjectionPointType(InjectionPointType::Iter, std::to_string(started));
    } 

    // Set the first values();
    wrapper->injectionPointStartedCallBack(comm, package, getScope(), type, stageId);
    
    InjectionPointBase::runTestsInternal(wrapper);    
    
    bool result = false;
    if (itIndex < m_iterators.size()) {
        int r = m_iterators[itIndex]->iterate_(comm, wrapper);
        if (r==0) {
            itIndex++;
        }
        result = itIndex < m_iterators.size();
    }
    wrapper->injectionPointEndedCallBack(getScope(), type, stageId);
    started++;

    if (result || once > started) {
      return true;
    } else {
      // No more iterations and meet the required minimum
      InjectionPointBase::setInjectionPointType(InjectionPointType::End, "End");
      InjectionPoint::run();
      return false;
    } 

    return result && once > started ;
}
