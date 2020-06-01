
/** @file UnitTestStore.cpp **/


#include <dlfcn.h>

#include <iostream>

#include "base/UnitTestStore.h"
#include "base/OutputEngineStore.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

UnitTestStore::UnitTestStore() {}

IUnitTest::IUnitTest(){};
IUnitTest::~IUnitTest(){};

UnitTestStore& UnitTestStore::getUnitTestStore() {
  static UnitTestStore store;
  return store;
}

void UnitTestStore::addUnitTester(std::string packageName, std::string name, tester_ptr m) {
  if ( tester_factory.find(packageName) == tester_factory.end()) {
      tester_factory[packageName] = {};
  }
  tester_factory[packageName][name] = m;
}

IUnitTest* UnitTestStore::getUnitTester(std::string packageName, std::string name) {
  auto it = tester_factory.find(packageName);
  if (it != tester_factory.end()) {
    auto itt = it->second.find(name);
    if ( itt != it->second.end() ) {
      return itt->second();
    }
  }
  return nullptr;
}

void UnitTestStore::runTest(VnV_Comm comm, std::string name, IUnitTest *tester) {
    OutputEngineManager *engineManager = OutputEngineStore::getOutputEngineStore().getEngineManager();
    engineManager->unitTestStartedCallBack(comm, name);
    tester->run();
    engineManager->unitTestFinishedCallBack(comm,tester);
}

void UnitTestStore::runTest(VnV_Comm comm, std::string packageName, std::string name) {
    auto it = tester_factory.find(packageName);
    if (it!=tester_factory.end()) {
        auto itt = it->second.find(name);
        if (itt != it->second.end()) {
            runTest(comm, packageName + ":" + name, itt->second());
        }
    }
}

void UnitTestStore::runPackageTests(VnV_Comm comm, std::string packageName) {
    auto it = tester_factory.find(packageName);
    if (it != tester_factory.end()) {
        for (auto &it : it->second) {
            runTest(comm, packageName + ":" + it.first, it.second());
        }
    }
}


void UnitTestStore::runAll(VnV_Comm comm, bool /*stopOnFail*/) {
    for (auto& it : tester_factory) {
        for (auto itt : it.second) {
            runTest(comm, it.first + ":" + itt.first, itt.second());
        }
    }
}

void UnitTestStore::print() {
    int a = VnV_BeginStage("Registered Unit Test Modules");
    for ( auto it : tester_factory ) {
        VnV_Info("Unit Test Module: %s ", it.first.c_str());
        auto aa = VnV_BeginStage("Test Suites");
        for ( auto itt : it.second) {
            VnV_Info("%s", itt.first.c_str());
        }
        VnV_EndStage(aa);
    }
    VnV_EndStage(a);
}

void VnV::registerUnitTester( std::string name, tester_ptr m) {
  UnitTestStore::getUnitTestStore().addUnitTester(PACKAGENAME_S, name, m);
}
