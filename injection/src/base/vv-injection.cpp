
/** @file vv-injection.cpp **/

#include "vv-injection.h"

#include <iostream>
#include "vv-logging.h"
#include "vv-output.h"
#include "vv-testing.h"

using namespace VnV;

InjectionPoint::InjectionPoint(std::string scope) : m_scope(scope) {}

std::string InjectionPoint::getScope() const { return m_scope; }

void InjectionPoint::addTest(TestConfig config) {
  std::shared_ptr<ITest> test = nullptr;
  test.reset(TestStore::getTestStore().getTest(config));

  VnV_Debug("Adding the Test {} " , test != nullptr );

  if (test != nullptr) {
    m_tests.push_back(test);
  }

}

bool InjectionPoint::hasTests() { return m_tests.size() > 0; }

void InjectionPoint::unpack_parameters(NTV& ntv, va_list argp) {

  while (1) {
    std::string variableType = va_arg(argp, char*);
    if (variableType == "__VV_END_PARAMETERS__") {
      break;
    }
    std::string variableName = va_arg(argp, char*);
    void* variablePtr = va_arg(argp, void*);
    ntv.insert(std::make_pair(variableName,
                    std::make_pair(variableType, variablePtr)));
  }
}

void InjectionPoint::runTests(int stageValue, va_list argp) {
  OutputEngineManager* wrapper =
      EngineStore::getEngineStore().getEngineManager();
  NTV ntv;
  unpack_parameters(ntv, argp);

  // Call the method to write this injection point to file.
  wrapper->startInjectionPoint(getScope(), stageValue);
  for (auto it : m_tests) {
      VnV_Debug("Running Test" ) ;   
	  it->_runTest(wrapper->getOutputEngine(), stageValue, ntv);
  }
  wrapper->endInjectionPoint(getScope(), stageValue);
}

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(
    std::string key) {
  auto it = injectionPoints.find(key);
  if (it != injectionPoints.end()) {
    std::shared_ptr<InjectionPoint> injectionPoint =
        std::make_shared<InjectionPoint>(it->first);
    
    VnV_Debug("Adding the Tests to injection point {} " , key );
    for (auto& test : it->second) {
      VnV_Debug("TestConfig {} {} ", test.getName(), test.getStages().size());
      injectionPoint->addTest(test);
    }
    return injectionPoint;
  }
  return nullptr;
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getInjectionPoint(
    std::string key, int stage) {

  VnV_Debug("Looking for injection point {} at stage {} ", key, stage );
    
  
  if (injectionPoints.find(key) == injectionPoints.end()) {
    VnV_Debug("Injection point {} not configured ", key );
    for ( auto it : injectionPoints ) {
	    VnV_Debug("{} {} {}" , it.first, key, key.compare(it.first));
    }
    
    
    return nullptr;  // Not configured
  } else if (stage == -1) {
    VnV_Debug("Returning new Injection point for stage -1 {} ", key );
    return newInjectionPoint(key); /* Single stage injection point --> Return an
                                      injection point directly. */
  } else if (stage ==
             0) { /* New staged injection point. -- Add it to the stack */
    auto it = active.find(key);
    auto ptr = newInjectionPoint(key); /*Not nullptr because we checked above*/

    /* If first of its kind, make a new map entry. */
    if (it == active.end()) {
      std::stack<std::shared_ptr<InjectionPoint>> s;
      s.push(ptr);
      active.insert(std::make_pair(key, s));
    } else {
      it->second.push(ptr);
    }
    return ptr;
  } else {
    // Stage > 0 --> Fetch the queue.
    auto it = active.find(key);
    if (it == active.end() || it->second.size() == 0) {
      return nullptr; /* queue doesn't exist or no active injection point -->
                         invalid. */
    } else if (stage == 9999) {
      auto iptr = it->second.top();  // Final stage, remove it from the stack.
      it->second.pop();
      return iptr;
    } else {
      return it->second.top();  // Intermediate stage -> return top of stack.
    }
  }
}

InjectionPointStore& InjectionPointStore::getInjectionPointStore() {
  static InjectionPointStore* store = new InjectionPointStore();
  return *store;
}

void InjectionPointStore::addInjectionPoint(std::string name,
                                            std::vector<TestConfig>& tests) {

    injectionPoints.insert(std::make_pair(name, tests));
}

void InjectionPointStore::addInjectionPoints(
    std::map<std::string, std::vector<TestConfig>>& injectionPoints) {
  this->injectionPoints.insert(injectionPoints.begin(), injectionPoints.end());
  for (auto i : injectionPoints) {
    VnV_Debug("Printing Tests for {}", i.first);
    for (auto it : i.second ) {
        VnV_Debug("Name {}" , it.getName());
        for (auto itt : it.getStages()) {
            VnV_Debug("Stage {}", itt.first);
        }
    }}
}
