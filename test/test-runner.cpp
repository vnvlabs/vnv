


#include "VnV.h"

#define PACKAGENAME VnVTestRunner

#ifdef WITH_MPI
  #include <mpi.h>
  INJECTION_EXECUTABLE(PACKAGENAME, VNV, mpi)
#else
  INJECTION_EXECUTABLE(PACKAGENAME, VNV, serial)
# define MPI_Init(...)
# define MPI_Finalize()
#endif

int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);

  INJECTION_INITIALIZE(PACKAGENAME, &argc, &argv,
                       (argc == 2) ? argv[1] : "./test-config.json");

  // Run The tests using the world communicator
  VnV_runUnitTests(VWORLD(PACKAGENAME));

  INJECTION_FINALIZE(PACKAGENAME);

#ifdef WITH_MPI
   MPI_Finalize();
#endif

}
