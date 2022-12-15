
#include <mpi.h>
#include <time.h>
#include <map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>

#include "VnV.h"

#define SPNAME SampleExecutable

//Register the executable with VNV
INJECTION_EXECUTABLE(SPNAME)

int main(int argc, char** argv) {
  
  
  /**
   * Sample Executable
   * =================
   *
   * This executable is designed to show a number of different injection points.
   * The first stage is to initialize the library. The text that appears in the
   * comment above the initialize call represents the introduction in the final
   * report.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, (argc == 2) ? argv[1] : "./inputfiles/injectionPoint.json");

  std::vector<int> samplePoints;
  
  /**
   *  A Simple Injection Point Loop
   *  -----------------------------
   *  
   *  This injection point loop tracks the values in the vector sample
   *  points. 

  **/
  INJECTION_LOOP_BEGIN(SPNAME, VSELF, Function1, VNV_NOCALLBACK, samplePoints);
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(SPNAME, Function1, "inner",VNV_NOCALLBACK);
  }
  INJECTION_LOOP_END(SPNAME, Function1,VNV_NOCALLBACK);
  

  INJECTION_FINALIZE(SPNAME);


}

