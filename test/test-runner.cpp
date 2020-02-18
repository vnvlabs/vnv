

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/iunittester.h"

void parser_callBack();

void callback() {
    parser_callBack();
}

int main(int argc, char** argv) {

    std::string configurationFile = "./test-config.json";
    if (argc == 1 ) {
        configurationFile = argv[1];
    }

    VnV_init(&argc,&argv,configurationFile.c_str(),callback);
    VnV_runUnitTests();
    VnV_finalize();
}

