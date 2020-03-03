
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined in InjectionPoint.h **/

#include <iostream>
#include "json-schema.hpp"
#include "base/Logger.h"
#include "base/OutputEngineStore.h"
#include "base/TestStore.h"
#include "base/InjectionPoint.h"
#include "c-interfaces/Injection.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/Logging.h"
using namespace VnV;
using nlohmann::json_schema::json_validator;

InjectionPoint::InjectionPoint(json registrationJson, NTV &args) {

     //RegistrationJson is a validated InjectionPoint Registration json object.
     m_scope = registrationJson["name"].get<std::string>();
     json parameters = registrationJson["parameters"];
     for ( const auto &it : parameters.items()) {
         auto p = args.find(it.key());
         void* ptr = (p == args.end()) ? nullptr : p->second.second;
         parameterMap[it.key()] = std::make_pair(it.value().get<std::string>(),ptr);
     }
     // Load any RTTI ("<none>" for C programs)
     for (const auto &it : args) {
         parameterRTTI[it.first] = it.second.first;
     }
 }

std::string InjectionPoint::getScope() const { return m_scope; }

std::string InjectionPoint::getParameterRTTI(std::string key)  const {
    auto it = parameterRTTI.find(key);
    if (it != parameterRTTI.end()) {
        return it->second;
    }
    return "<Unknown>";
}

void InjectionPoint::addTest(TestConfig config) { 
     std::shared_ptr<ITest> test = nullptr;
     test.reset(TestStore::getTestStore().getTest(config));
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
      it->_runTest(wrapper->getOutputEngine(), type, stageId, parameterMap);
  }
  wrapper->injectionPointEndedCallBack(getScope(), type,stageId);
}


