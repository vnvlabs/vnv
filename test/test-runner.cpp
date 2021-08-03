#include "VnV.h"

#define PACKAGENAME VnVTestRunner


//Generally an application will either use MPI or it wont use MPI.
//We include this switch here for testing purposes so we can use this
//application with and without MPI.

// In a real application, mpi.h will already be included (because MPI_Init will
// have to be called somehow. So, you will only need to call the injection executable
// command.

INJECTION_EXECUTABLE(PACKAGENAME)

int main(int argc, char** argv) {


  INJECTION_INITIALIZE(PACKAGENAME, &argc, &argv,
                       (argc == 2) ? argv[1] : "./test-config.json");


  INJECTION_FINALIZE(PACKAGENAME);


}
