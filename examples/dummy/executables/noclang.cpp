
#include "VnV.h"


/**
 * 
 * Shows how a user can initialize VnV executable without having to 
 * run the clang tool over it. In this case, the user cannot have any 
 * injectoin points and tests. 
 * 
 */

#define SPNAME NoClangExecutable

INJECTION_EXECUTABLE_NOCLANG(SPNAME,HYPRE)

//Just a dummy
INJECTION_REGISTRATION(HYPRE){};


int main(int argc, char** argv) {
  
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, "./vv-input.json");
  
  // ... Here you would make calls to subpackages that use vnv. 
  int x = 10;

  INJECTION_FINALIZE(SPNAME);

}
