
#include <mpi.h>
#include "vv-runtime.h"
#include <iostream>
#include <vector>
#include "dlclass1.h"
#include "class1.h"
#include "class2.h"
#include "dlclass2.h"

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

    MPI_Init(&argc,&argv);

    VV_init("sample-tests.xml");
  
    std::cout << "Calling a function that has injection points\n";
    function1(10);

    class1 sample_class_1;
    class2 sample_class_2;

    dummyLibOne::class1 sample_class_3;
    dummyLibOne::class1 sample_class_4;

    sample_class_1.function1(10);
    sample_class_2.function1(10);
    sample_class_3.function1(10);
    sample_class_4.function1(10);



    VV_finalize();
}

