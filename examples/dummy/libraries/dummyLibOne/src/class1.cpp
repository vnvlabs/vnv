
#include "VnV.h"
#include "dlclass1.h"

using namespace dummyLibOne;

int class1::function1(int x) {
  double slope = 2;
  double intersection = 3;
  double value = 0;

  INJECTION_LOOP_BEGIN(VNV_STR(DLPNAME), VSELF, "Hello_temp_sub", slope,
                       intersection, x, value);

  value = slope * x + intersection;

  INJECTION_LOOP_END(VNV_STR(DLPNAME), "Hello_temp_sub");

  return static_cast<int>(value);
}
