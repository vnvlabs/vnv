
#ifndef VV_UNITTESTING_HEADER
#define VV_UNITTESTING_HEADER

#include <map>
#include <string>

#include "VnV-Interfaces.h"

namespace VnV {

class UnitTestStore {
 private:
  std::map<std::string, tester_ptr*, std::less<std::string>> tester_factory;
  UnitTestStore();

 public:
  void addUnitTester(std::string name, tester_ptr m);

  IUnitTester* getUnitTester(std::string name);

  static UnitTestStore& getUnitTestStore();

  void runAll(bool stopOnFail);
};

}  // namespace VnV

#endif
