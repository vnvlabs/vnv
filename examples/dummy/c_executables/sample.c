#include "VnV.h"
#include <stdio.h>
#ifdef __cplusplus
  #warning "Compiling C example with a C++ compiler."
#endif
#define PNAME SampleCExecutable

int function1(int x) {

  INJECTION_LOOP_BEGIN(PNAME, VWORLD(PNAME), CFunction, x)
  for (int i = 0; i < 10; i++) {
    x += i;
    INJECTION_LOOP_ITER(PNAME,CFunction, inner);
  }

  INJECTION_LOOP_END(PNAME,CFunction);
  return x;
}

static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

INJECTION_OPTIONS(PNAME,schemaCallback){

}


#ifdef WITH_MPI
   #include <mpi.h>
   INJECTION_EXECUTABLE(PNAME,mpi)
#else
  #define MPI_Init(...)
  #define MPI_Finalize()
  INJECTION_EXECUTABLE(PNAME, VNV, serial)
#endif

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  INJECTION_INITIALIZE(PNAME, &argc, &argv, (argc==2) ? argv[1] : "./sample.json");
  function1(10);
  INJECTION_FINALIZE(PNAME);
  MPI_Finalize();

}
