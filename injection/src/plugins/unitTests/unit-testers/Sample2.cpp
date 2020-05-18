
#ifndef Sample2_H
#define Sample2_H

#include "VnV.h"
#include "interfaces/IUnitTester.h"

#include <sstream>
using namespace VnV;

/**
  A sample unit test.
*/
INJECTION_UNITTEST(Sample2) {
  std::map<std::string,bool> results;
  results["true"] = true;
  results["!false"] = !false;
  return results;
}
#endif
