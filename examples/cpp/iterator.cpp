
#include <mpi.h>
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>

#include "VnV.h"

#define SPNAME SimpleExecutable
INJECTION_EXECUTABLE(SPNAME)

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /**
   * Simple Executable
   * =================
   *
   * This executable evaluates the function
   *     f = 10x^2 -200 .
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, (argc == 2) ? argv[1] : "./inputfiles/iterator.json");

  double f = 0;
  double x = 0;

  /**
   * Function evaluation for  f = 10x^2 - 200.
   * -----------------------------------------
   *
   */
  INJECTION_ITERATION(zx, VNV_STR(SPNAME), VSELF, "FunctionEvaluation", 1, x, f) { f = 10 * x * x - 200; }

  /**
     Conclusion.
     ===========

     Whatever we put here will end up in the conclusion.
  */
  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
