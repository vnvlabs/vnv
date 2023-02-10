
/** @file UnitTestStore.cpp **/

#include "base/stores/UnitTestStore.h"

#include <dlfcn.h>

#include <iostream>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/Logging.h"

using namespace VnV;

BaseStoreInstance(UnitTestStore)

    UnitTestStore::UnitTestStore() {}

IUnitTest::IUnitTest() {}
IUnitTest::~IUnitTest() {}

ICommunicator_ptr UnitTestStore::dispatch(VnV_Comm comm, int cores) {
  auto c = CommunicationStore::instance().getCommunicator(comm);
  if (c->Rank() == 0) {
    if (cores > c->Size()) {
      VnV_Warn_MPI(VNVPACKAGENAME, VSELF, "Test Requested %d cores but only %d are available -- skipping", cores,
                   c->Size());
      return nullptr;
    }
  }
  return c;
}

nlohmann::json UnitTestStore::schema(json& packageJson) {
  nlohmann::json m = R"({"type":"object"})"_json;
  nlohmann::json props = json::object();
  for (auto& it : tester_factory) {
    nlohmann::json m1 = R"({"type":"object"})"_json;
    nlohmann::json props1 = json::object();

    for (auto& itt : it.second) {
      props1[itt.first] = R"({"type" : "boolean"})"_json;
      props1[itt.first]["vnvprocs"] = tester_cores[it.first + ":" + itt.first];
      props1[itt.first]["description"] = packageJson[it.first]["UnitTests"][itt.first]["docs"]["description"];
    }
    m1["properties"] = props1;
    props[it.first] = m1;
  }
  m["properties"] = props;
  m["additionalProperties"] = false;
  return m;
}

void UnitTestStore::addUnitTester(std::string packageName, std::string name, tester_ptr m, int cores) {
  if (tester_factory.find(packageName) == tester_factory.end()) {
    tester_factory[packageName] = {};
  }
  tester_factory[packageName][name] = m;
  tester_cores[packageName + ":" + name] = cores;
}

IUnitTest* UnitTestStore::getUnitTester(std::string packageName, std::string name) {
  auto it = tester_factory.find(packageName);
  if (it != tester_factory.end()) {
    auto itt = it->second.find(name);
    if (itt != it->second.end()) {
      return itt->second();
    }
  }
  return nullptr;
}

void UnitTestStore::runTest(ICommunicator_ptr comm, std::string packageName, std::string name, IUnitTest* tester) {
  tester->setComm(comm);
  OutputEngineManager* engineManager = OutputEngineStore::instance().getEngineManager();
  engineManager->unitTestStartedCallBack(comm, packageName, name);
  tester->run();
  engineManager->unitTestFinishedCallBack(tester);
}

void UnitTestStore::runTest(ICommunicator_ptr comm, std::string packageName, std::string name) {
  auto it = tester_factory.find(packageName);
  if (it != tester_factory.end()) {
    auto itt = it->second.find(name);
    if (itt != it->second.end()) {
      runTest(comm, packageName, name, itt->second());
    }
  }
}

// This algorithm runs all the tests in parallel (kindof) by creating
// a subset of that spans the provided comm as best as possible.

// TODO A master slave system would probably be better. In this one,
// communicators have no way of knowing when all the other processors
// are finished. The tests are just allocated sequentially based on
// the available ranks and the remaining tests.

void UnitTestStore::runAll(VnV_Comm comm, VnV::UnitTestInfo info) {
  if (!info.runUnitTests) {
    return;
  }

  auto c = CommunicationStore::instance().getCommunicator(comm);
  int size = c->Size();
  int rank = c->Rank();
  // Transform the tess vector of tuples and sort largest to smallest.
  std::vector<std::tuple<int, std::string, std::string, tester_ptr>> tests;
  for (auto it : tester_factory) {
    std::set<std::string> blist;
    if (info.unitTestConfig.contains(it.first)) {
      json& pconfig = info.unitTestConfig[it.first];
      if (pconfig.contains("runUnitTests") && !pconfig["runUnitTests"].get<bool>()) {
        // run unit tests was false --> so we continue without adding any from
        // this pacakge.
        continue;
      }
      if (pconfig.contains("unitTests")) {
        for (auto& n : pconfig["unitTests"].items()) {
          if (!n.value().get<bool>()) {
            blist.insert(n.key());
          }
        }
      }
    }

    for (const auto& itt : it.second) {
      if (blist.find(itt.first) == blist.end()) {
        std::string key = it.first + ":" + itt.first;
        int cores = tester_cores[key];

        tests.push_back(std::make_tuple(cores, it.first, itt.first, itt.second));
      }
    }
  }

  std::sort(tests.begin(), tests.end(),
            [](const std::tuple<int, std::string, std::string, tester_ptr>& p1,
               const std::tuple<int, std::string, std::string, tester_ptr>& p2) {
              return std::get<0>(p1) > std::get<0>(p2);
            });

  auto it = tests.begin();
  int currentBuffer = 0;
  int myStart = -1;
  int myEnd = -1;
  std::string myName;
  std::string myPackage;
  tester_ptr myTester;
  while (tests.size() > 0) {
    if (it == tests.end()) {
      it = tests.begin();
    }

    int req = std::get<0>(*it);  // requested processors for this iter.
    std::string name = std::get<2>(*it);
    std::string pname = std::get<1>(*it);
    if (req > size) {
      VnV_Warn(VNVPACKAGENAME, "Ignoring test %s because cannot fufull requested cores of %d", name.c_str(), req);
      it = tests.erase(it);
    } else if (currentBuffer + req <= size) {
      if (rank >= currentBuffer && rank < currentBuffer + req) {
        myStart = currentBuffer;
        myEnd = currentBuffer + req;
        myName = name;
        myPackage = pname;
        myTester = std::get<3>(*it);
      }

      it = tests.erase(it);  // erase it because its done.
      currentBuffer += req;
    } else {
      it++;  // move on to the next one.
    }
    // If smallest left woould fill the buffer, then run. If there are no
    // tests left, then the smallest left is set to size to trigger a run.
    int smallestLeft = (tests.size() == 0) ? size : std::get<0>(tests.back());

    if (currentBuffer + smallestLeft > size) {
      if (myStart >= 0) {
        // Create comm - collective on the ranks [myStart,myEnd).
        ICommunicator_ptr p = c->create(myStart, myEnd, 1, 10);
        auto pcomm = CommunicationStore::instance().toVnVComm(p);
        VnV_Debug_MPI(VNVPACKAGENAME, pcomm, "Running %s %s on range [%d,%d)", myName.c_str(), myPackage.c_str(),
                      myStart, myEnd);
        runTest(p, myPackage, myName, (*myTester)());
      }
      currentBuffer = 0;
      myStart = -1;
      myEnd = -1;
    } else {
    }
  }
  c->Barrier();  //
}

void UnitTestStore::print() {
  for (auto it : tester_factory) {
    VnV_Info(VNVPACKAGENAME, "Unit Test Module: %s ", it.first.c_str());
    for (auto itt : it.second) {
      VnV_Info(VNVPACKAGENAME, "%s", itt.first.c_str());
    }
  }
}

void VnV::registerUnitTester(std::string packageName, std::string name, tester_ptr m, int cores) {
  UnitTestStore::instance().addUnitTester(packageName, name, m, cores);
}
