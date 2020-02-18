

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/iunittester.h"

void parser_callBack();

VnV::IUnitTester* parser_maker();

void callback() {
    parser_callBack();
}

int main(int argc, char** argv) {
    VnV_init(&argc,&argv,"./test-config.json",callback);
    VnV_runUnitTests();
    VnV_finalize();
}

