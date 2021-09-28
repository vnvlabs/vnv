
#include <mpi.h>
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>

#include "VnV.h"


#define SPNAME Templates

template <typename T> class f {
 public:
  T ff;
  T gg;
  int ggg;
  int getF(int t) {
    /** sdfsdfsdfsdf **/
    INJECTION_POINT(T, VNV_STR(SPNAME), VSELF, "templateClass", ff, gg, t);
    return 1;
  
  
    VnV_Injection<T>("PACKAGE", VSELF, "NAME")
  
  }
};


template<typename T, typename U, typename V>
int templateFnc(T x, U y, V z) {
  INJECTION_POINT(VNV_STR(SPNAME), VSELF, "templatefnc", x,y,z);
 
  return 1;
}


INJECTION_EXECUTABLE(SPNAME)

int main(int argc, char** argv) {
  
  INJECTION_INITIALIZE(SPNAME, &argc, &argv,
                       (argc == 2) ? argv[1] : "./vv-input.json");


  f<double> templateClass1;
  templateClass1.getF(1);

  f<int> templateClass2;
  templateClass2.getF(1);

  templateFnc(1, 1.0, "sdfsdf");
  templateFnc(1, "", "");

  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();


  

}

