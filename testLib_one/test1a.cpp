
#include "test1.h"



int test1a::function1(int x) {
      #define test1a_function1_T_VVTest int,x
      INJECTION_POINT(test1a_function1_T, 0)
      INJECTION_POINT(test1a_function1_T, 9999)
      return x;
}
REGISTER_IP(test1a_function1_T, 0,  "Sample Test")
REGISTER_IP(test1a_function1_T, 9999,  "Sample Test")


