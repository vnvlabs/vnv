
#ifndef Sample2_H
#define Sample2_H

#include <sstream>

#include "VnV.h"
using namespace VnV;

/**
  A sample unit test.
*/
INJECTION_UNITTEST(VNVPACKAGENAME, Sample2, 1) {
  TEST_ASSERT_EQUALS("true", true, true);
  TEST_ASSERT_EQUALS("false", false, false);
  TEST_ASSERT_EQUALS("not true", !true, false);
}
#endif
