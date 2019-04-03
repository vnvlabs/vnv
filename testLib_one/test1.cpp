
#include "test1.h"

int test1::function1(int x) {
      #define Hello_temp_x_VVTest int,x
      INJECTION_POINT(Hello_temp_x, 0)
      INJECTION_POINT(Hello_temp_x, 9999)
      return x;
}

REGISTER_IP(Hello_temp_x, 0, "Sample Test End")
REGISTER_IP(Hello_temp_x, 9999, "Sample Testi End")

