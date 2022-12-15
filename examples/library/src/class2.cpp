
#include "VnV.h"
#include "dlclass1.h"
#include "dlclass2.h"
using namespace dummyLibOne;

int class2::function1(int x) {
  INJECTION_POINT(DLPNAME, VSELF, class2_function1, VNV_NOCALLBACK, x);
  return x;
}
