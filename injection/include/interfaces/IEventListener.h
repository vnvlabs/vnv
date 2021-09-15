#ifndef INTERFACES_IEVENTLISTENER_H
#  define INTERFACES_IEVENTLISTENER_H

#  include <string>
#  include <type_traits>

#  include "base/Communication.h"
#  include "base/exceptions.h"
#  include "base/stores/CommunicationStore.h"
#  include "base/stores/DataTypeStore.h"
#  include "base/stores/ReductionStore.h"
#  include "c-interfaces/Communication.h"
#  include "c-interfaces/Logging.h"
#  include "c-interfaces/PackageName.h"
#  include "c-interfaces/Wrappers.h"
#  include "interfaces/IUnitTest.h"
#  include "json-schema.hpp

using nlohmann::json;

namespace VnV {

class IEventListener : {
 public:
  virtual void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string id,
                                             InjectionPointType type, std::string stageId, std::string filename,
                                             int line) = 0;

  virtual void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageId) = 0;

  virtual void testStartedCallBack(std::string packageName, std::string testName, bool internal, long uuid) = 0;

  virtual void testFinishedCallBack(bool result_) = 0;

  virtual void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string unitTestName) = 0;

  virtual void unitTestFinishedCallBack(IUnitTest* tester) = 0;

  virtual void packageOptionsStartedCallBack(ICommunicator_ptr comm, std::string packageName) = 0;
  virtual void packageOptionsEndedCallBack(std::string packageName) = 0;

  virtual ~IEventListener() = default;
};

}  // namespace VnV