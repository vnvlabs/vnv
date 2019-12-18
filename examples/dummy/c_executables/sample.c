
#include "VnV.h"

#ifdef __cplusplus 
  #warning "Compiling C example with a C++ compiler." 
#endif


int function1(int x) {

  INJECTION_LOOP_BEGIN(CFunction, int x)
  for (int i = 0; i < 10; i++) {
    x += i;	    
    INJECTION_LOOP_ITER(CFunction, inner, double, x)
  }

  INJECTION_LOOP_END(CFunction, double, x)
  return x;
};

void callback() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back from C executable");  
};

int main(int argc, char** argv) {
  VnV_init(&argc, &argv, "./sample.json",callback);

  function1(10);
 
  VnV_finalize();
}
