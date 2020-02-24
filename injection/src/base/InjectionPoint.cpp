
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined in InjectionPoint.h **/

#include <iostream>
#include "json-schema.hpp"
#include "base/Logger.h"
#include "base/OutputEngineStore.h"
#include "base/TestStore.h"
#include "base/InjectionPoint.h"
#include "c-interfaces/Injection.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

InjectionPoint::InjectionPoint(std::string scope) : m_scope(scope) {}

std::string InjectionPoint::getScope() const { return m_scope; }

void InjectionPoint::addTest(TestConfig config) {
  std::shared_ptr<ITest> test = nullptr;
  test.reset(TestStore::getTestStore().getTest(config));


  if (test != nullptr) {
    m_tests.push_back(test);
  }

}

void InjectionPoint::setInjectionPointType(InjectionPointType type_, std::string stageId_) {
    type = type_;
    stageId = stageId_;
}

bool InjectionPoint::hasTests() { return m_tests.size() > 0; }

void InjectionPoint::unpack_parameters(NTV& ntv, va_list argp) {

  while (1) {
    std::string variableType = va_arg(argp, char*);
    if (variableType == VNV_END_PARAMETERS_S) {
      break;
    }
    std::string variableName = va_arg(argp, char*);
    void* variablePtr = va_arg(argp, void*);
    auto it = ntv.find(variableName);
    if (it!=ntv.end()) {
        // If nullptr, indicates this variable is no longer available, so remove it.
        if (variablePtr==NULL) {
            ntv.erase(variableName);
        } else {
            // Variable has been defined before. In this case, replace the entry to it.
            it->second = std::make_pair(variableType,variablePtr);
        }
    } else {
        // Put a new entry into the NTV object.
        ntv.insert(std::make_pair(variableName,std::make_pair(variableType,variablePtr)));
    }
  }
}

void InjectionPoint::runTests(va_list argp) {
  OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();

  // Parameter map is this injection points parameter map. The pointers to the injection objects are
  // stored across injection point calls. So, in staged calls, it is up to the developer to ensure
  // the pointers are up to date and still active.
  unpack_parameters(parameterMap,argp);

  // Call the method to write this injection point to file.

  wrapper->injectionPointStartedCallBack(getScope(), type, stageId);
  for (auto it : m_tests) {
      it->_runTest(wrapper->getOutputEngine(), type, stageId, parameterMap);
  }
  wrapper->injectionPointEndedCallBack(getScope(), type,stageId);
}


