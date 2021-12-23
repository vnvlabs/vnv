#ifndef INTERFACES_IEVENTLISTENER_H
#define INTERFACES_IEVENTLISTENER_H

#include <string>
#include <type_traits>


#include "c-interfaces/Communication.h"
#include "c-interfaces/Logging.h"
#include "c-interfaces/PackageName.h"
#include "c-interfaces/Wrappers.h"
#include "interfaces/IUnitTest.h"
#include "json-schema.hpp"

using nlohmann::json;


namespace VnV {



class IEventListener {
public:

  virtual void initialize(ICommunicator_ptr world) {};

  virtual void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string id,
                                             InjectionPointType type, std::string stageId, std::string filename,
                                             int line) {};

  virtual void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageId) {};

  virtual void testStartedCallBack(std::string packageName, std::string testName, bool internal, long uuid) {};

  virtual void testFinishedCallBack(bool result_) {};

  virtual void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string unitTestName) {};

  virtual void unitTestFinishedCallBack(IUnitTest* tester) {};

  virtual void finalize(ICommunicator_ptr world) {}



  virtual ~IEventListener() = default;
};

}  // namespace VnV

#endif