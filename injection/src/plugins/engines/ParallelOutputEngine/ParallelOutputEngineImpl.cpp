/** @file vv-debug-engine.cpp **/

#include "plugins/engines/ParallelOutputEngine/ParallelOutputEngineImpl.h"

#include <unistd.h>

#include <iostream>

#include "base/CommunicationStore.h"
#include "base/Utilities.h"
#include "c-interfaces/Logging.h"
namespace VnV {
namespace VNVPACKAGENAME {
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

void ParallelEngine::Put(std::string variableName,

                         const double& value) {
  std::string str = std::to_string(value);
  Put( variableName, str);
}
void ParallelEngine::Put(std::string variableName,
                         const int& value) {
  std::string str = std::to_string(value);
  Put( variableName, str);
}
void ParallelEngine::Put( std::string variableName,
                         const long& value) {
  std::string str = std::to_string(value);
  Put( variableName, str);
}

void ParallelEngine::Put( std::string variableName,
                         const bool& value) {
  std::string str = std::to_string(value);
  Put( variableName, str);
}

void ParallelEngine::Put(std::string variableName,
                         const json& value) {
  Put( variableName, value.dump());
}

void ParallelEngine::Put( std::string variableName,
                         const std::string& value) {
  getRouter(RouterAction::PUSH)->send(variableName, value);
}

void ParallelEngine::Log(ICommunicator_ptr comm, const char* package, int stage,
                         std::string level, std::string message) {
  currComm = comm;
  if (getRouter(RouterAction::IGNORE)->isRoot()) {
    std::string s = VnV::StringUtils::getIndent(stage);
    printf("%s[%s:%s]: %s\n", s.c_str(), package, level.c_str(),
           message.c_str());
  }
}

json ParallelEngine::getConfigurationSchema() {
  return __parallel_engine_schema__;
}

void ParallelEngine::finalize() {
  VnV_Info(VNVPACKAGENAME, "PARALLEL ENGINE: FINALIZE");
}

void ParallelEngine::setFromJson(json& config) {
  printf("PARALLEL ENGINE WRAPPER Init with file %s\n", config.dump().c_str());
  // router = new Router();
}

void ParallelEngine::injectionPointEndedCallBack(ICommunicator_ptr comm, std::string id,
                                                 InjectionPointType type,
                                                 std::string stageVal) {
  printf("PARALLEL ENGINE End Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
  getRouter(RouterAction::POP)->forward();
}

void ParallelEngine::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                   std::string packageName,
                                                   std::string id,
                                                   InjectionPointType type,
                                                   std::string stageVal) {
  currComm = comm;
  printf("PARALLEL ENGINE Start Injection Point %s : %s \n", id.c_str(),
         InjectionPointTypeUtils::getType(type, stageVal).c_str());
}

void ParallelEngine::testStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                                         std::string testName, bool internal) {
  currComm = comm;
  printf("PARALLEL ENGINE Start Test %s \n", testName.c_str());
}

void ParallelEngine::testFinishedCallBack(ICommunicator_ptr comm, bool result_) {
  printf("PARALLEL ENGINE Stop Test. Test Was Successful-> %d\n", result_);
}
void ParallelEngine::dataTypeStartedCallBack(ICommunicator_ptr  comm ,
                                             std::string variableName,
                                             long long dtype) {
  currComm = comm;
  printf("PARALLEL ENGINE Data Type Started %s", variableName.c_str());
}
void ParallelEngine::dataTypeEndedCallBack(ICommunicator_ptr /** comm **/,
                                           std::string variableName) {
  printf("PARALLEL ENGINE Data Type Finished %s ", variableName.c_str());
}

void ParallelEngine::unitTestStartedCallBack(ICommunicator_ptr comm,
                                             std::string packageName,
                                             std::string unitTestName) {
  currComm = comm;
  printf("PARALLEL ENGINE START UNIT TEST: %s\n", unitTestName.c_str());
}

void ParallelEngine::unitTestFinishedCallBack(ICommunicator_ptr comm,
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

std::shared_ptr<Router> ParallelEngine::getRouter(
                                                  RouterAction action) {
  auto id = currComm->uniqueId();
  auto rit = routerMap.find(id);
  if (rit != routerMap.end()) {
    auto result = rit->second;
    if (action == RouterAction::POP) {
      routerMap.erase(rit);
    }
    return result;
  }

  std::shared_ptr<Router> r(new Router(currComm));
  if (action == RouterAction::PUSH) {
    routerMap.insert(std::make_pair(id, r));
  }
  return r;
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV

INJECTION_ENGINE(VNVPACKAGENAME, Parallel) {
  return new VnV::VNVPACKAGENAME::Engines::ParallelEngine();
}
