﻿#include "VnV.h"

#define PACKAGENAME VnVTestRunner

INJECTION_EXECUTABLE(PACKAGENAME)

const char* inputfile = R"(

{
  "runTests": true,
  "outputEngine": {
    "json_file" : {
       "filename" : "test-results"
    }
  },
  "unit-testing" : {
     "runUnitTests" : true
  }
}

)"; 

int main(int argc, char** argv) {
  INJECTION_INITIALIZE_RAW(PACKAGENAME, &argc, &argv, inputfile);
  INJECTION_FINALIZE(PACKAGENAME);
}
