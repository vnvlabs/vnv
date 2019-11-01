
#include "VnV.h"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    VnV_init(&argc,&argv,"./test-config.json");
    VnV_runUnitTests();
    VnV_finalize();
}

