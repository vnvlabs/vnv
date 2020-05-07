

#include <iostream>
#include <vector>

#include "VnV.h"

#include "interfaces/IUnitTester.h"

void parser_callBack();
void BFSTester_callBack(); 
void sampleTest_callBack(); 
void Sample2_callBack(); 
//<callback-declare-needle>


namespace VnV { 
  namespace Registration { 

  void RegisterBuiltinUnitTests() {
    parser_callBack();
    BFSTester_callBack(); 
    sampleTest_callBack(); 
    Sample2_callBack(); 
    //<callback-generation-needle> 
  }
 }
}


