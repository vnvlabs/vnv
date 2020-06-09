


#include "VnV.h"

#ifdef WITH_MPI
  #include <mpi.h>
  INJECTION_COMMUNICATOR(mpi)
#endif

int main(int argc, char** argv) {

#ifdef WITH_MPI
    MPI_Init(&argc, &argv);
#endif

  INJECTION_INITIALIZE(&argc, &argv,
                       (argc == 2) ? argv[1] : "./test-config.json");
  // Run The tests using the world communicator defined for this package. In
  // this case,
  VnV_runUnitTests(VWORLD);
  INJECTION_FINALIZE();

#ifdef WITH_MPI
   MPI_Finalize();
#endif

}
