
#include "VnV.h"
#include <stdio.h>
#ifdef __cplusplus
  #warning "Compiling C example with a C++ compiler."
#endif


int function1(int x) {

  INJECTION_LOOP_BEGIN(VnV_Comm_Self, CFunction, x)
  for (int i = 0; i < 10; i++) {
    x += i;
    INJECTION_LOOP_ITER(CFunction, inner);
  }

  INJECTION_LOOP_END(CFunction);
  return x;
}

static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

INJECTION_OPTIONS(schemaCallback) {

}

INJECTION_REGISTRATION() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back from C executable");
   REGISTER_OPTIONS
   Register_Injection_Point("CFunction","{\"x\":\"int\"}");
}

int main(int argc, char** argv) {

  INJECTION_INITIALIZE(&argc, &argv, (argc==2) ? argv[1] : "./sample.json");
  function1(10);
  INJECTION_FINALIZE();
}
