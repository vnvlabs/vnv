
#include "test1.h"

class LinearFunction {

  public:
    double slope, intersection;

    LinearFunction(double slope_, double intersection_) :
      slope(slope_), intersection(intersection_) {}

    double eval(double x) {
      double value = slope*x + intersection;
      //INJECTION_POINT("IP_1",-1,
      //                double slope,
      //                double intersection,
      //                double x,
      //                double value);
      return value;
    }
};
//REGISTER_IP("IP_1",-1,double slope,double intersection,
  //           double x, double value);



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
