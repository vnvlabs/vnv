
/** @file vv-injection.cpp **/

#include <iostream>
#include "json-schema.hpp"
#include "base/Logger.h"
#include "base/OutputEngineStore.h"
#include "base/TestStore.h"
#include "base/InjectionPoint.h"
#include "c-interfaces/injection-point-interface.h"

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
    ntv.insert(std::make_pair(variableName,
                    std::make_pair(variableType, variablePtr)));
  }
}

void InjectionPoint::runTests(va_list argp) {
  OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();
  NTV ntv;
  unpack_parameters(ntv, argp);

  // Call the method to write this injection point to file.

  wrapper->injectionPointStartedCallBack(getScope(), type, stageId);
  for (auto it : m_tests) {
      it->_runTest(wrapper->getOutputEngine(), type, stageId, ntv);
  }
  wrapper->injectionPointEndedCallBack(getScope(), type,stageId);
}


