

#include "vv-runtime.h"
#include <iostream>
#include <vector>
#include "test1.h"
#include "test2.h"

int function1(int x) {
    std::vector<double> samplePoints(10), samplePoints1(10);
    #define ErrorTest_VVTest std::vector<double>,samplePoints, std::vector<double>,samplePoints1
    INJECTION_POINT(ErrorTest, 0)
    for ( int i = 0; i < 10; i++ ) {
      samplePoints.push_back(i);
      samplePoints1.push_back(i*i);
    }
    INJECTION_POINT(ErrorTest, 9999)
    return 11;
}
REGISTER_IP(ErrorTest, 0, "Sample Test End")
REGISTER_IP(ErrorTest, 9999, "Sample Test End")

int main(int argc, char** argv) {

   // MPI_Init(&argc,&argv);

    VV_init("sample-tests.xml");
  
    function1(10);

    std::cout << "Calling a function that has injection points\n";
    function1(10);
    test1 t;
    test1a tt;
    t.function1(10);
    tt.function1(10);

    c_test_function(2234);

    VV_finalize();
}

