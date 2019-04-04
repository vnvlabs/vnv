

#include "injection.h"
#include "test1.h"
#include "test2.h"

int function1(int x) {
    return 11;
}

int main(int argc, char** argv) {

    MPI_Init(&argc,&argv);

    VV::VVInit("sample-tests.xml");


    std::cout << "Calling a function that has injection points\n";
    function1(10);
    test1 t;
    test1a tt;
    t.function1(10);
    tt.function1(10);

    c_test_function(2234);

    VV::VVFinalize();
}

