
#include "VnV.h"
#include <mpi.h>

#define SPNAME DakotaExample

INJECTION_EXECUTABLE(SPNAME, VNV, mpi)

int main(int argc, char** argv) {

 MPI_Init(&argc, &argv);
 /**
   * Dakota Example
   * =================
   *
   * This executable deomstrates using the dakota tests at a VnV Iteration
   * point.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, (argc == 2) ? argv[1] : "./sample.json");

  double x = 10;
  double y = 20;
  double f = 0;

  /**
   * Simple Function evaluation of the function
   *
   * ..math::
   *
   *     f(x,y) = (x+y)^2 + 100x - 10y
   *
   *
   */
  INJECTION_ITERATION(SampleApplication, VWORLD, dakotaLoop, 1, 2, x , y, f ) {
     f = (x + y)*(x + y) + 100*x - 10*y;
  }

  /**
     Conclusion.
     ===========

     That concludes the Dakota example -- I hope you enjoyed the show.
  */
  INJECTION_FINALIZE(SPNAME);




  MPI_Finalize();
}
