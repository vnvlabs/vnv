
/** @file InjectionPoint.cpp Implementation of InjectionPoint class as defined
 * in InjectionPoint.h **/

#include <iostream>

#include "base/Logger.h"
#include "base/points/InjectionPoint.h"
#include "base/stores/ActionStore.h"
#include "base/stores/InjectionPointStore.h"
#include "base/stores/IteratorStore.h"
#include "base/stores/IteratorsStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/TestStore.h"
#include "common-interfaces/Logging.h"
#include "interfaces/IIterator.h"
#include "json-schema.hpp"

using namespace VnV;

using nlohmann::json_schema::json_validator;

void IterationPoint::addIterator(IteratorConfig& config) {
  config.setParameterMap(parameterMap);
  std::shared_ptr<IIterator> test = IteratorsStore::instance().getTest(config);

  if (test != nullptr) {
    m_iterators.push_back(test);
    return;
  }

  VnV_Error(VNVPACKAGENAME, "Error Loading Iterator with Name %s", config.getName().c_str());
}

bool IterationPoint::iterate(std::string filename, int line, const DataCallback& callback) {
  OutputEngineManager* wrapper = OutputEngineStore::instance().getEngineManager();

  // On the first iteration, call INjectionPointBegin and run all the tests.
  if (started == 0) {
    InjectionPointBase::setInjectionPointType(InjectionPointType::Begin, "Begin");
    InjectionPoint::run(filename, line, callback);
    started++;
    return true;
  } else {
    InjectionPointBase::setInjectionPointType(InjectionPointType::Iter, std::to_string(started));
  }

  // Set the first values();
  wrapper->injectionPointStartedCallBack(comm, package, getName(), type, stageId, filename, line);

  InjectionPointBase::runTestsInternal(wrapper,callback);

  bool result = false;
  if (itIndex < m_iterators.size()) {
    int r = m_iterators[itIndex]->iterate_(comm, wrapper);
    if (r == 0) {
      itIndex++;
    }
    result = itIndex < m_iterators.size();
  }
  wrapper->injectionPointEndedCallBack(getName(), type, stageId);
  started++;

  if (result || once > started) {
    return true;
  } else {
    // No more iterations and meet the required minimum
    InjectionPointBase::setInjectionPointType(InjectionPointType::End, "End");
    InjectionPoint::run(filename, line,callback);
    return false;
  }

  return result && once > started;
}
