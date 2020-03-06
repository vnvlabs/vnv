
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined in InjectionPoint.h **/

#include <iostream>
#include "json-schema.hpp"
#include "base/Logger.h"
#include "base/OutputEngineStore.h"
#include "base/TestStore.h"
#include "base/InjectionPoint.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

using nlohmann::json_schema::json_validator;

InjectionPoint::InjectionPoint(json registrationJson, NTV &args) {

     //RegistrationJson is a validated InjectionPoint Registration json object.
     m_scope = registrationJson["name"].get<std::string>();
     json parameters = registrationJson["parameters"];

     for (auto it : args ) {
         auto rparam = parameters.find(it.first); // Find a parameter with this name.
         if (rparam != parameters.end()) {
            parameterMap[it.first] = VnVParameter(it.second.second, rparam.value().get<std::string>(),it.second.first);
         } else {
            VnV_Warn("Injection Point %s is not configured Correctly. Unrecognized parameter %s", m_scope.c_str(), it.first.c_str());
            parameterMap[it.first] = VnVParameter(it.second.second, "void*", it.second.first);
         }
    }
 }

std::string InjectionPoint::getScope() const { return m_scope; }

std::string InjectionPoint::getParameterRTTI(std::string key)  const {
    auto it = parameterMap.find(key);
    if (it!=parameterMap.end()) {
        return it->second.getRtti();
    }
    return "";
}

void InjectionPoint::addTest(TestConfig config) { 
     config.setParameterMap(parameterMap);
     std::shared_ptr<ITest> test = TestStore::getTestStore().getTest(config);
     
     if (test != nullptr) {
        m_tests.push_back(test);
        return;
     }
     VnV_Error("Error Loading Test Config with Name %s", config.getName().c_str());
}


void InjectionPoint::setInjectionPointType(InjectionPointType type_, std::string stageId_) {
    type = type_;
    stageId = stageId_;
}

bool InjectionPoint::hasTests() { return m_tests.size() > 0; }


void InjectionPoint::runTests() {
  OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();
  // Call the method to write this injection point to file.

  wrapper->injectionPointStartedCallBack(getScope(), type, stageId);
  for (auto it : m_tests) {
      it->_runTest(wrapper->getOutputEngine(), type, stageId );
  }
  wrapper->injectionPointEndedCallBack(getScope(), type,stageId);
}


