

#include <iostream>
#include <vector>

#include "../injection/include/interfaces/IUnitTest.h"
#include "VnV.h"

void register_parser_tests();
void register_bitvector_tests();

void callback() {
    register_parser_tests();
    register_bitvector_tests();
}

int main(int argc, char** argv) {
    VnV_init(&argc,&argv, (argc==2) ? argv[1] : "./test-config.json",callback);
    VnV_runUnitTests();
    VnV_finalize();
}

