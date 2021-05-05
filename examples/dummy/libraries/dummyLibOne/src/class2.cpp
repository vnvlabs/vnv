
#include "VnV.h"
#include "dlclass1.h"
#include "dlclass2.h"
using namespace dummyLibOne;

int class2::function1(int x) {

  INJECTION_POINT(VNV_STR(DLPNAME), VSELF, "class2_function1", x);
  return x;
}
