
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
   * Example Iteration point. This example iterator evaluates the function
   *
   * TODO [vnv-math] (x + y)*(x+y) + 100*x - 10*y
   *
   * TODO: Plot the function + mathjax to display the function.
   * TODO: Add a default callback option that writes the data automatically
   * for use in this comment.
   *
   */
  INJECTION_ITERATION(SPNAME, VWORLD(SPNAME), dakotaLoop, 1, 2, x , y, f ) {
     f = (x + y)*(x + y) + 100*x - 10*y;
     std::cout << "( f, x, y) : " << f << " " << x << " " << y << std::endl;
  }


  /**
     Conclusion.
     ===========

     That concludes the Dakota example -- I hope you enjoyed the show.
  */
  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
