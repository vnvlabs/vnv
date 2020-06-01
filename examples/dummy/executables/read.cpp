
#include <iostream>
#include <vector>

#include "VnV.h"


int main(int argc, char** argv) {

  INJECTION_INITIALIZE(&argc, &argv, (argc==2) ? argv[1] : "./vv-input.json");
  VnV_readFile("./vv-output.json");
  INJECTION_FINALIZE()

}
