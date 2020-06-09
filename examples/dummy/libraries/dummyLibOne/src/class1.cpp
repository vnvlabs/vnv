
#include "VnV.h"
#include "dlclass1.h"

using namespace dummyLibOne;

int class1::function1(int x) {
  double slope = 2;
  double intersection = 3;
  double value = 0;

  auto a = VnV_BeginStage(
      "An example of a third party library using the VnV logging capabilities");

  INJECTION_LOOP_BEGIN(VSELF, Hello_temp_sub, slope, intersection, x, value);

  value = slope * x + intersection;

  INJECTION_LOOP_END(Hello_temp_sub);

  VnV_EndStage(a);

  return static_cast<int>(value);
}
