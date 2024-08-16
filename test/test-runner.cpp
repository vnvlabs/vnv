#include "VnV.h"
#define PACKAGENAME VnVTestRunner

INJECTION_EXECUTABLE(PACKAGENAME,"{}")

const char* inputfile = R"(

{
  "runTests": true,
  "outputEngine": {
    "file" : {
       "filename" : "test-results"
    }
  },
  "unit-testing" : {
     "runUnitTests" : true
  }
}

)"; 

int main(int argc, char** argv) {
 try {
  INJECTION_INITIALIZE(PACKAGENAME, &argc, &argv);
  INJECTION_FINALIZE(PACKAGENAME);
 } catch (const char * e) {
  std::cout << e << std::endl;
 }
}
