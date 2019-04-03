
#include "test1_two.h"



int two::test1::function1(int x) {
      #define TestTwo_VVTest int,x
      INJECTION_POINT(TestTwo, 0);
      INJECTION_POINT(TestTwo, 9999);
      return x;
  }
REGISTER_IP(TestTwo,0,"Test in lib two")
REGISTER_IP(TestTwo,9999,"Test in lib two")



