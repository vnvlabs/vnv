

#include <iostream>
#include <vector>

#include "VnV.h"

#include "interfaces/IUnitTester.h"

void parser_callBack();
void BFSTester_callBack(); 
//<callback-declare-needle>


void callback() {
    parser_callBack();
    BFSTester_callBack(); 
    //<callback-generation-needle> 
}

int main(int argc, char** argv) {
    VnV_init(&argc,&argv, (argc==2) ? argv[1] : "./test-config.json",callback);
    VnV_runUnitTests();
    VnV_finalize();
}

