
#include "VnV.h"
#include "dlclass2.h"

using namespace dummyLibOne;

int class2::function1(int x) {
#define class2_function1_VVTest int, x
  INJECTION_POINT(class2_function1, 0)
  INJECTION_POINT(class2_function1, 9999)
  return x;
}
