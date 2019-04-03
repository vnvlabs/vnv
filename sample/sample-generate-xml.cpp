

#include "injection.h"
#include "test1.h"
#include "test1_two.h"

int function1(int x) {
    return 11;
}

int main(int argc, char** argv) {

    MPI_Init(&argc,&argv);

    
    VV::writeXMLFile("sssssss.xml");
    

    std::cout << "Calling a function that has injection points\n";
    function1(10);
    test1 t;
    test1a tt;
    two:test1 t2a;
    two::test1a ta;
    t.function1(10);
    tt.function1(10);
    ta.function1(100); 

}

