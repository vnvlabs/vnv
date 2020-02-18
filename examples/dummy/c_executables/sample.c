
#include "VnV.h"
#include <stdio.h>
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
}


char* schemaCallback() {
    return "{\"type\": \"object\", \"required\":[\"option1\"]}";
}

void optionsCallback(c_json json) {

    VnV_printJson(json);
    char * opt;
    c_json temp;
    VnV_getKeyValue(json, "option3",&temp);
    VnV_getString(temp, &opt);
    printf("%s sdfsdfsdf ", opt);
    VnV_freeString(temp,&opt);

}

void callback() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back from C executable");    
   VnV_Register_Options(schemaCallback, optionsCallback);

}

int main(int argc, char** argv) {
  VnV_init(&argc, &argv, "./sample.json",callback);

  function1(10);
 
  VnV_finalize();
}
