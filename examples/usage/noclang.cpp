
#include "VnV.h"
#include "dlclass1.h"

/**
 * 
 * Shows how a user can initialize VnV executable without having to 
 * run the clang tool over it. In this case, the user cannot have any 
 * injectoin points and tests. 
 * 
 */

#define SPNAME NoClangExecutable

// Here we define a executable called SPNAME that includes links 
// to a list of VNV equit libraries. In this example, we link to 
// DummyLibOne.   
INJECTION_EXECUTABLE_NOCLANG(SPNAME,DummyLibOne)

int main(int argc, char** argv) {
  
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, "./input.json");
  
  //Here you would make calls to subpackages that use vnv. 
  dummyLibOne::class1 a;
  a.function1(10);


  INJECTION_FINALIZE(SPNAME);

}
