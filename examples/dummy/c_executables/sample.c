#include "VnV.h"
#include <stdio.h>
#ifdef __cplusplus
  #warning "Compiling C example with a C++ compiler."
#endif


#define PNAME SampleCExecutable

int function1(int x) {

struct VnV_Function_Sig a;
a.compiler = "";
a.signiture = "";

  INJECTION_LOOP_BEGIN(VNV_STR(PNAME), VWORLD, "CFunction", x)
  for (int i = 0; i < 10; i++) {
    x += i;
    INJECTION_LOOP_ITER(VNV_STR(PNAME),"CFunction", "inner");
  }

  INJECTION_LOOP_END(VNV_STR(PNAME),"CFunction");
  return x;
}

static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

INJECTION_OPTIONS(PNAME,schemaCallback){

}

INJECTION_EXECUTABLE(PNAME)

int main(int argc, char** argv) {

  INJECTION_INITIALIZE(PNAME, &argc, &argv, (argc==2) ? argv[1] : "./sample.json");
  function1(10);
  INJECTION_FINALIZE(PNAME);

}
