
/**
  @file UnitTestStore.h
**/
#ifndef VV_UNITTESTING_HEADER
#define VV_UNITTESTING_HEADER

#include <map>
#include <string>

#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IUnitTest.h"

namespace VnV {

class UnitTestStore : public BaseStore {
 private:
  std::map<std::string,
           std::map<std::string, tester_ptr, std::less<std::string>>>
      tester_factory;

  std::map<std::string, int> tester_cores;

  void runTest(ICommunicator_ptr comm, std::string packageName,
               std::string Name, IUnitTest* tester);

  ICommunicator_ptr dispatch(VnV_Comm comm, int cores);

 public:
  UnitTestStore();

  void addUnitTester(std::string packageName, std::string name, tester_ptr m,
                     int cores);

  IUnitTest* getUnitTester(std::string packageName, std::string name);

  void runTest(ICommunicator_ptr comm, std::string packageName,
               std::string testName);

  void runAll(VnV_Comm comm, VnV::UnitTestInfo info);

  void print();

  json schema();

  static UnitTestStore& instance();
};

}  // namespace VnV

#endif
