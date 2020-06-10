/** @file vv-debug-engine.cpp **/

#include "plugins/engines/ParallelOutputEngine/ParallelOutputEngineImpl.h"
#include "c-interfaces/Logging.h"
#include <iostream>
#include <unistd.h>

#include <iostream>

#include "base/Utilities.h"
#include "base/CommunicationStore.h"
#include "c-interfaces/Logging.h"
namespace VnV {
namespace PACKAGENAME {
namespace Engines {

static json __parallel_engine_schema__ =
    R"(
{
	"$schema": "http://json-schema.org/draft-07/schema#",
	"$id": "http://rnet-tech.net/vv-debug-schema.json",
	"title": "Parallel Engine Input Schema",
	"description": "Schema for the parallel engine",
    "type": "object"
}
)"_json;

ParallelEngine::ParallelEngine() {}

void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const double& value) {
  std::string str = std::to_string(value);
  Put(comm, variableName, str);
}
void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const int& value) {
  std::string str = std::to_string(value);
  Put(comm, variableName, str);
}
void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const float& value) {
  std::string str = std::to_string(value);
  Put(comm, variableName, str);
}
void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const long& value) {
  std::string str = std::to_string(value);
  Put(comm, variableName, str);
}

void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const bool& value) {
  std::string str = std::to_string(value);
  Put(comm, variableName, str);
}

void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const json& value) {
  Put(comm, variableName, value.dump());
}

void ParallelEngine::Put(VnV_Comm comm, std::string variableName,
                         const std::string& value) {
  getRouter(comm,RouterAction::PUSH)->send(variableName, value);
}

void ParallelEngine::Log(VnV_Comm comm, const char* package, int stage,
                         std::string level, std::string message) {
  if (getRouter(comm,RouterAction::IGNORE)->isRoot()) {
    std::string s = VnV::StringUtils::getIndent(stage);
    printf("%s[%s:%s]: %s\n", s.c_str(), package, level.c_str(),
           message.c_str());
  }
}

json ParallelEngine::getConfigurationSchema() {
  return __parallel_engine_schema__;
}

void ParallelEngine::finalize() { VnV_Info("PARALLEL ENGINE: FINALIZE"); }

void ParallelEngine::setFromJson(json& config) {
  printf("PARALLEL ENGINE WRAPPER Init with file %s\n", config.dump().c_str());
  //router = new Router();
}

void ParallelEngine::injectionPointEndedCallBack(VnV_Comm comm, std::string id,
                                                 InjectionPointType type,
                                                 std::string stageVal) {
  printf("PARALLEL ENGINE End Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
  getRouter(comm,RouterAction::POP)->forward();
}

void ParallelEngine::injectionPointStartedCallBack(VnV_Comm comm,
                                                   std::string id,
                                                   InjectionPointType type,
                                                   std::string stageVal) {
  printf("PARALLEL ENGINE Start Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
}

void ParallelEngine::testStartedCallBack(VnV_Comm comm, std::string testName) {
  printf("PARALLEL ENGINE Start Test %s \n", testName.c_str());
}

void ParallelEngine::testFinishedCallBack(VnV_Comm comm, bool result_) {
  printf("PARALLEL ENGINE Stop Test. Test Was Successful-> %d\n", result_);
}

void ParallelEngine::unitTestStartedCallBack(VnV_Comm comm,
                                             std::string unitTestName) {
  printf("PARALLEL ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
}

void ParallelEngine::unitTestFinishedCallBack(VnV_Comm comm,
                                              IUnitTest* tester) {
  printf("Test Results\n");
  bool suiteSuccessful = true;
  for (auto it : tester->getResults()) {
    printf("\t%s : %s\n", std::get<0>(it).c_str(),
           std::get<2>(it) ? "Successful" : "Failed");
    if (!std::get<2>(it)) {
      printf("\t\t%s\n", std::get<1>(it).c_str());
      suiteSuccessful = false;
    }
  }
  printf("PARALLEL ENGINE Test Suite Completed : %s\n",
         (suiteSuccessful) ? "Successfully" : "Unsuccessfully");
}

std::shared_ptr<Router> ParallelEngine::getRouter(VnV_Comm comm_raw, RouterAction action) {
  auto comm = VnV::CommunicationStore::instance().getCommunicator(comm_raw);
  auto rit = routerMap.find(comm->uniqueId());
  if (rit != routerMap.end()) {
      auto result = rit->second;
      if (action == RouterAction::POP) {
         routerMap.erase(rit);
      }
      return result;
  }

  std::shared_ptr<Router> r(new Router(comm));
  if (action == RouterAction::PUSH) {
     routerMap.insert(std::make_pair(comm->uniqueId(),r));
  }
  return r;
}

}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV

INJECTION_ENGINE(Parallel) {
  return new VnV::PACKAGENAME::Engines::ParallelEngine();
}
