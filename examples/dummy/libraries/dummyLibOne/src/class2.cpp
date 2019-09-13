
#include "dlclass2.h"

using namespace dummyLibOne;

int class2::function1(int x) {
      #define class2_function1_VVTest int,x
      INJECTION_POINT(class2_function1, 0)
      INJECTION_POINT(class2_function1, 9999)
      return x;
}
REGISTER_IP(class2_function1, 0,  "Sample Test")
REGISTER_IP(class2_function1, 9999,  "Sample Test")


