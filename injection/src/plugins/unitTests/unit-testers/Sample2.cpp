
#ifndef Sample2_H
#define Sample2_H

#include "VnV.h"

#include <sstream>
using namespace VnV;

/**
  A sample unit test.
*/
INJECTION_UNITTEST(Sample2) {
  TEST_ASSERT_EQUALS("true",true,true);
  TEST_ASSERT_EQUALS("false",false,false);
  TEST_ASSERT_EQUALS("not true", !true,false);
}
#endif
