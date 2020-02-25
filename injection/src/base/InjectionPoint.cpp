
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

InjectionPoint::InjectionPoint(json registrationJson, va_list args) {

     //RegistrationJson is a validated InjectionPoint Registration json object.
     m_scope = registrationJson["name"].get<std::string>();
     json parameters = registrationJson["parameters"];
     for ( auto it : parameters.items()) {
         std::string pname = it.key();
         json parameter = it.value();

         // add the parameter to the ntv map;
         parameterMap[pname] = std::make_pair(parameter["class"].get<std::string>(),nullptr);
     }
     // Load up the ntv with the args.
     unpack_parameters(parameterMap,args);
}

std::string InjectionPoint::getScope() const { return m_scope; }


void InjectionPoint::addTest(TestConfig config) {
  if ( config.isMappingValidForParameterSet(parameterMap)) {

     std::shared_ptr<ITest> test = nullptr;
     test.reset(TestStore::getTestStore().getTest(config));
     if (test != nullptr) {
        m_tests.push_back(test);
        return;
     }
  }
  VnV_Error("Error Loading Test Config with Name %s", config.getName().c_str());
}


void InjectionPoint::setInjectionPointType(InjectionPointType type_, std::string stageId_) {
    type = type_;
    stageId = stageId_;
}

bool InjectionPoint::hasTests() { return m_tests.size() > 0; }

void InjectionPoint::unpack_parameters(NTV& ntv, va_list argp) {

  while (1) {
    std::string variableName = va_arg(argp, char*);
    if (variableName == VNV_END_PARAMETERS_S) {
      break;
    }
    void* variablePtr = va_arg(argp, void*);

    auto it = ntv.find(variableName);
    if (it!=ntv.end()) {
        it->second.second = variablePtr;
    } else {
        //variable was not registered, add it with a type void*
        ntv.insert(std::make_pair(variableName, std::make_pair("void*", variablePtr)));
    }
  }
}

void InjectionPoint::runTests() {
  OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();
  // Call the method to write this injection point to file.

  wrapper->injectionPointStartedCallBack(getScope(), type, stageId);
  for (auto it : m_tests) {
      it->_runTest(wrapper->getOutputEngine(), type, stageId, parameterMap);
  }
  wrapper->injectionPointEndedCallBack(getScope(), type,stageId);
}


