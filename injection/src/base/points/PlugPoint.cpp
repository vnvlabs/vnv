
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined
 * in InjectionPoint.h **/

#include "base/points/PlugPoint.h"

#include <iostream>

#include "base/stores/InjectionPointStore.h"
#include "base/Logger.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/TestStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
#include "interfaces/IIterator.h"
#include "interfaces/IPlug.h"
#include "base/stores/TestStore.h"
#include "base/stores/PlugsStore.h"


using namespace VnV;

using nlohmann::json_schema::json_validator;

void PlugPoint::setPlug(PlugConfig &config) {
  config.setParameterMap(parameterMap);
  m_plug = PlugsStore::instance().getTest(config);
}

bool PlugPoint::plug() {
    
    if (started) {
      // This happens when we dont plug so we can finalize any tests that ran at the end of the plug
      InjectionPointBase::setInjectionPointType(InjectionPointType::End,"End");
      InjectionPoint::run();
      return false;
    }

    OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();
    InjectionPointBase::setInjectionPointType(InjectionPointType::Begin,"Begin");
    
    started = true;    
    
    
    wrapper->injectionPointStartedCallBack(comm, package, getScope(), type, stageId);

    // Run the tests before setting the values.    
    InjectionPointBase::runTestsInternal(wrapper);

    //Run the plug (sets the values. )
    bool result = false;
    if (m_plug != nullptr) {
      result = m_plug->plug_(comm,wrapper); 
    }

    wrapper->injectionPointEndedCallBack(getScope(), type, stageId);
    
    if (result) {
      //Finalize the tests after the plug. 
      InjectionPointBase::setInjectionPointType(InjectionPointType::End,"End");
      InjectionPoint::run();
    }
    
    return result;
}
