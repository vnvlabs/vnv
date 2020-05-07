

#include <iostream>
#include <vector>

#include "VnV.h"

#include "interfaces/IUnitTester.h"

//<callback-declare-needle>

void callback() {
    //<callback-generation-needle> 
}

int main(int argc, char** argv) {
    VnV_init(&argc,&argv, (argc==2) ? argv[1] : "./test-config.json",callback);
    VnV_runUnitTests(VnV_Comm_World);
    VnV_finalize();
}

