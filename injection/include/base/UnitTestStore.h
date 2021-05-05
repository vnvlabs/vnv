
/**
  @file UnitTestStore.h
**/
#ifndef VV_UNITTESTING_HEADER
#define VV_UNITTESTING_HEADER

#include <map>
#include <string>

#include "JsonParser.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IUnitTest.h"

namespace VnV {

class UnitTestStore {
 private:
  std::map<std::string,
           std::map<std::string, tester_ptr*, std::less<std::string>>>
      tester_factory;

  std::map<std::string, int> tester_cores;

  UnitTestStore();

  void runTest(Communication::ICommunicator_ptr comm, std::string packageName,
               std::string Name, IUnitTest* tester);

  Communication::ICommunicator_ptr dispatch(VnV_Comm comm, int cores);

 public:
  void addUnitTester(std::string packageName, std::string name, tester_ptr m,
                     int cores);

  IUnitTest* getUnitTester(std::string packageName, std::string name);

  static UnitTestStore& getUnitTestStore();

  void runTest(Communication::ICommunicator_ptr comm, std::string packageName,
               std::string testName);

  void runAll(VnV_Comm comm, VnV::UnitTestInfo info);

  void print();
};

}  // namespace VnV

#endif
