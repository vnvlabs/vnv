
#include "VnV.h"
#include "dlclass2.h"

using namespace dummyLibOne;

int class2::function1(int x) {

  INJECTION_POINT(VnV_Comm_Self, class2_function1,x);

  return x;
}
