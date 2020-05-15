
/**
  @file UnitTestStore.h
**/
#ifndef VV_UNITTESTING_HEADER
#define VV_UNITTESTING_HEADER

#include <map>
#include <string>

#include "../interfaces/IUnitTest.h"

namespace VnV {

class UnitTestStore {
 private:
  std::map<std::string, std::map<std::string, tester_ptr*, std::less<std::string>>> tester_factory;
  UnitTestStore();

  void runTest(std::string Name, IUnitTest *tester);

 public:
  void addUnitTester(std::string packageName, std::string name, tester_ptr m);

  IUnitTest* getUnitTester(std::string packageName, std::string name);

  static UnitTestStore& getUnitTestStore();


  void runTest(std::string packageName, std::string testName);

  void runPackageTests(std::string packageName);

  void runAll(bool stopOnFail);

  void print();

};

}  // namespace VnV

#endif
