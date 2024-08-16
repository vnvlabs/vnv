
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined
 * in InjectionPoint.h **/


#include <iostream>

#include "base/InjectionPoint.h"
#include "base/Logger.h"
#include "base/stores/InjectionPointStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/SamplerStore.h"
#include "base/stores/TestStore.h"
#include "common-interfaces/all.h"
#include "validate/json-schema.hpp"
#include "shared/exceptions.h"

using namespace VnV;

InjectionPointBase::InjectionPointBase(std::string packageName, std::string name,
                                       std::map<std::string, std::string> registrationJson, const NTV& args) {
  this->name = name;
  this->package = packageName;

  for (auto it : args) {
    auto rparam = registrationJson.find(it.first);  // Find a parameter with this name.
    if (rparam != registrationJson.end()) {
      parameterMap.insert(std::make_pair(it.first, VnVParameter(it.second.second, rparam->second)));
    } else {
      VnV_Warn(VNVPACKAGENAME,
               "Injection Point is not configured Correctly. Unrecognized "
               "parameter "
               "%s",
               it.first.c_str());
      parameterMap.insert(std::make_pair(it.first, VnVParameter(it.second.second, "void*")));
    }
  }
}

void InjectionPointBase::addTest(TestConfig& config) {
  config.setParameterMap(parameterMap);
  std::shared_ptr<ITest> test = TestStore::instance().getTest(config);

  if (test != nullptr) {
    m_tests.push_back(test);
    return;
  }
  VnV_Error(VNVPACKAGENAME, "Error Loading Test Config with Name %s", config.getName().c_str());
}

void InjectionPointBase::setInjectionPointType(InjectionPointType type_, std::string stageId_) {
  type = type_;
  stageId = stageId_;
}

void InjectionPointBase::setComm(ICommunicator_ptr comm) { this->comm = comm; }


void InjectionPointBase::runTestsInternal(OutputEngineManager* wrapper, const DataCallback& callback) {
  try {
    wrapper->testStartedCallBack(package, "__internal__", true, -1);
    try {
      auto a = VnVCallbackData(comm->get(), parameterMap, wrapper, type, stageId);
      callback(a);
    } catch (VnVExceptionBase& e) {
      VnV_Error(VNVPACKAGENAME, "Error Running internal Test: %s", e.what());
    }
    wrapper->testFinishedCallBack(true);  // TODO callback should return bool "__internal__");

    for (auto it : m_tests) {
      it->_runTest(comm, wrapper, type, getName() + ":" + stageId);
    }



  } catch (VnVExceptionBase& e) {
    VnV_Error(VNVPACKAGENAME, "Exception occured when running tests");
  }

}

void InjectionPoint::runChild(std::string function, int line, InjectionPoint& child) {
   if (skipped) {
    return; //We were skipped so dont run it.
   }

   OutputEngineManager* wrapper = OutputEngineStore::instance().getEngineManager();      
   wrapper->injectionPointStartedCallBack(comm, package, getName(), InjectionPointType::Child_Iter, child.stageId, function, line);
      
    //We don't store the callback for the internal, so we cant call that. Note: we could store it, but stuff would go out of scope
    //way to much and this would be bad. We require that the pointers in an injection point loop are valid for the entire loop, so 
    //we are safe to run any tests defined on the injection point. But the callback is two dangerous at the moment. 
    //runTestsInternal(wrapper, callback);
      
    for (auto it : m_tests) {
        it->_runTest(comm, wrapper, InjectionPointType::Child_Iter, child.getName() + ":" + child.stageId);
    }

    wrapper->injectionPointEndedCallBack(getName(), type, stageId);

}

void InjectionPoint::run(std::string function, int line, const DataCallback& callback) {
  if (skipped) {
    return;  // We are marked as skipped, so don't do anything.
  }

  // We always run Injection point end -- you cant skip that if begin was
  // called,
  bool runIt = (type == InjectionPointType::End);

  if (!runIt) {

    // The injection point only runs IF the sampler lets it. This lets us
    // completly skip injection points or iterations. The sampler can say to
    // skip iters, Begin, and single.
    auto sampler = SamplerStore::instance().getSamplerForInjectionPoint(package, name);
    runIt = sampler == nullptr ? true : sampler->sample(comm, type, stageId);
  }

  if (runIt) {
    OutputEngineManager* wrapper = OutputEngineStore::instance().getEngineManager();

    wrapper->injectionPointStartedCallBack(comm, package, getName(), type, stageId, function, line);

    runTestsInternal(wrapper, callback);

    wrapper->injectionPointEndedCallBack(getName(), type, stageId);

    //If this is Stage "End", then we have already been popped off the injection point stack. So,
    //The last parameter, when false, will skip the check for ourselves. 
    auto parents = InjectionPointStore::instance().getParents(*this, (type != InjectionPointType::End));
    
    for (auto &it : parents) {
      it->runChild(function, line,*this);
    }

  } else if (type == InjectionPointType::Begin) {
    // If we didn;t run and this is a BEGIN, then we set our skipped property
    // because we don't want to run any injection point iters and the end call.
    skipped = true;
  }

  
}
