
/** @file vv-unit-tester.cpp **/

#include "vv-unit-tester.h"

#include <dlfcn.h>

#include <iostream>

#include "vv-runtime.h"

using namespace VnV;

UnitTestStore::UnitTestStore() {}

IUnitTester::IUnitTester(){};
IUnitTester::~IUnitTester(){};

UnitTestStore& UnitTestStore::getUnitTestStore() {
  static UnitTestStore* store = new UnitTestStore();
  return *store;
}

void UnitTestStore::addUnitTester(std::string name, tester_ptr m) {
  tester_factory[name] = m;
}

IUnitTester* UnitTestStore::getUnitTester(std::string name) {
  auto it = tester_factory.find(name);
  if (it != tester_factory.end()) {
    return it->second();
  }
  return nullptr;
}

void UnitTestStore::runAll(bool /*stopOnFail*/) {
  for (auto& it : tester_factory) {
    IUnitTester* tester = it.second();
    RunTime::instance().loadInjectionPoints(tester->getInputJson());
    tester->run();
  }
}

void UnitTestStore::print() {
    VnV_BeginStage("Registered Unit Test Modules");
    for ( auto it : tester_factory ) {
        VnV_Info("Unit Test Module: %s ", it.first.c_str());
    }
    VnV_EndStage("");
}

void VnV_registerUnitTester(std::string name, tester_ptr m) {
  UnitTestStore::getUnitTestStore().addUnitTester(name, m);
}
