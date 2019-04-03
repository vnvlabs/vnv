

#include "injection.h"
#include "test1.h"
#include "test1_two.h"

int function1(int x) {
    return 11;
}

int main(int argc, char** argv) {

    MPI_Init(&argc,&argv);

    std::cout << "Printing out all registerd Injection Points\n"; 
    //InjectionPointBaseFactory::getAll();
    VV::VVInit("sample-tests.xml");


    std::cout << "Calling a function that has injection points\n";
    function1(10);
    test1 t;
    test1a tt;
    two:test1 t2a;
    two::test1a ta;
    t.function1(10);
    tt.function1(10);
    ta.function1(100); 

    VV::VVFinalize();
}

