
#include "VnV.h"
#include <stdio.h>
#ifdef __cplusplus 
  #warning "Compiling C example with a C++ compiler." 
#endif


int function1(int x) {

  INJECTION_LOOP_BEGIN(CFunction, x)
  for (int i = 0; i < 10; i++) {
    x += i;	    
    INJECTION_LOOP_ITER(CFunction, inner)
  }

  INJECTION_LOOP_END(CFunction)
  return x;
}


static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

void optionsCallback(c_json json) {

}

// Write the injection point registration json. A bit yuck in C.
static const char* injectionPoints = "\
{\
 \"name\" : \"CFunction\",\
 \"parameters\" : {\
   \"x\" : \"int\"\
 }\
}";

void callback() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back from C executable");    
   VnV_Register_Options(schemaCallback, optionsCallback);
   Register_Injection_Point(injectionPoints);
}

int main(int argc, char** argv) {

  if (argc !=2 )
    VnV_init(&argc, &argv, "./sample.json",callback);
  else {
    VnV_init(&argc,&argv, argv[1], callback);
  }

  function1(10);
 
  VnV_finalize();
}
