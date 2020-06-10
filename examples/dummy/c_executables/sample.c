#include "VnV.h"
#include <stdio.h>
#ifdef __cplusplus
  #warning "Compiling C example with a C++ compiler."
#endif


int function1(int x) {

  INJECTION_LOOP_BEGIN(VWORLD, CFunction, x)
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

#ifdef WITH_MPI
   #include <mpi.h>
   INJECTION_COMM(mpi)
#else
  #define MPI_Init(...)
  #define MPI_Finalize()
#endif

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  INJECTION_INITIALIZE(&argc, &argv, (argc==2) ? argv[1] : "./sample.json");
  function1(10);
  INJECTION_FINALIZE();
  MPI_Finalize();

}
