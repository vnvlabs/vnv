
#include "test1.h"


int test1::function1(int x) {
      double y = 34;
      float z = 10;
      // Define macro Name _VVTest int,x,double,y,float,z
      
      #define Hello_temp_x_VVTest int,x
      INJECTION_POINT(Hello_temp_x, 0)

      z += 30;
      y += 333;
      x += 22;  
        

      #define Hello_temp_sub_VVTest int,x
      INJECTION_POINT(Hello_temp_sub, 0)
      INJECTION_POINT(Hello_temp_sub, 9999)


      INJECTION_POINT(Hello_temp_x, 9999)
      
      
      return x;
}

// This only really works for C++ / not C
REGISTER_IP(Hello_temp_x, 0, "Sample Test End")
REGISTER_IP(Hello_temp_x, 9999, "Sample Testi End")
REGISTER_IP(Hello_temp_sub, 0, "Sample Test End")
REGISTER_IP(Hello_temp_sub, 9999, "Sample Testi End")
