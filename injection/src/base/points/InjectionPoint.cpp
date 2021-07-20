
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



using namespace VnV;

using nlohmann::json_schema::json_validator;


InjectionPointBase::InjectionPointBase(std::string packageName, std::string name,
                               json registrationJson, const NTV& in_args, const NTV& out_args) {
  this->name = name;
  this->package = packageName;

  for (int arg_switch = 0; arg_switch<2; arg_switch++) {
    bool inputs = (arg_switch==0);
    const NTV& args = (inputs) ? in_args : out_args;

    for (auto it : args) {
        auto rparam = registrationJson.find(it.first);  // Find a parameter with this name.
        if (rparam != registrationJson.end()) {
             parameterMap.insert(std::make_pair(it.first, VnVParameter(
                 it.second.second, rparam.value().get<std::string>(), it.second.first, inputs)));
        } else {
          VnV_Warn(
              VNVPACKAGENAME,
              "Injection Point is not configured Correctly. Unrecognized parameter "
              "%s",
              it.first.c_str());
              parameterMap.insert(std::make_pair(it.first,
                  VnVParameter(it.second.second, "void*", it.second.first,inputs)));
        }
     }
  }
}

std::string InjectionPointBase::getScope() const { return name; }

std::string InjectionPointBase::getParameterRTTI(std::string key) const {
  auto it = parameterMap.find(key);
  if (it != parameterMap.end()) {
    return it->second.getRtti();
  }
  return "";
}

void InjectionPointBase::addTest(TestConfig &config) {
  config.setParameterMap(parameterMap);
  std::shared_ptr<ITest> test = TestStore::instance().getTest(config);

  if (test != nullptr) {
    m_tests.push_back(test);
    return;
  }
  VnV_Error(VNVPACKAGENAME, "Error Loading Test Config with Name %s",
            config.getName().c_str());
}



void InjectionPointBase::setInjectionPointType(InjectionPointType type_, std::string stageId_) {
  type = type_;
  stageId = stageId_;
}


void InjectionPointBase::setCallBack(injectionDataCallback* callback) {
  if (runInternal && cCallback != nullptr) {
    cCallback = callback;
    callbackType = 1;
  }
}
void InjectionPointBase::setCallBack(const DataCallback& callback) {
  if (runInternal && callback != nullptr) {
    cppCallback = callback;
    callbackType = 2;
  }
}

void InjectionPointBase::setComm(ICommunicator_ptr comm) { this->comm = comm; }

void InjectionPointBase::runTestsInternal(OutputEngineManager *wrapper) {

    if (callbackType > 0) {
      wrapper->testStartedCallBack( package, "__internal__", true, -1);
      if (callbackType == 1) {
        IOutputEngineWrapper engineWraper = {
            static_cast<void*>(wrapper->getOutputEngine())};
        ParameterSetWrapper paramWrapper = {static_cast<void*>(&parameterMap)};
        int t = InjectionPointTypeUtils::toC(type);
        (*cCallback)(comm->asComm(), &paramWrapper, &engineWraper, t, stageId.c_str());
      } else {
        cppCallback(comm->asComm(), parameterMap, wrapper, type, stageId);
      }
      wrapper->testFinishedCallBack(true);  // TODO callback should return bool "__internal__");
    }
    for (auto it : m_tests) {
      it->_runTest(comm, wrapper, type, stageId);
    }

}

/********************* INJECTION POINT *********************************/

void InjectionPoint::run() {
  OutputEngineManager* wrapper = OutputEngineStore::getOutputEngineStore().getEngineManager();

  wrapper->injectionPointStartedCallBack(comm, package, getScope(), type, stageId);
  runTestsInternal(wrapper);
  wrapper->injectionPointEndedCallBack(getScope(), type, stageId);

}

