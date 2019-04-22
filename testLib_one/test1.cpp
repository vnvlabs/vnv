
#include "test1.h"


int test1::function1(int x) {
      
      double slope = 2;
      double intersection = 3; 
      double value = 0;
        
      #define Hello_temp_sub_VVTest double,slope,double,intersection,double,value
      INJECTION_POINT(Hello_temp_sub, 0)
      
      value = slope*x + intersection;
      
      INJECTION_POINT(Hello_temp_sub, 9999)
      
      
      return value;
}

REGISTER_IP(Hello_temp_sub, 0, "Sample Test End")
REGISTER_IP(Hello_temp_sub, 9999, "Sample Test End")
