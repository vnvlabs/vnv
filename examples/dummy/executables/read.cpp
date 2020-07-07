
#include <iostream>
#include <vector>

#include "VnV.h"

int main(int argc, char** argv) {
  INJECTION_INITIALIZE(&argc, &argv, (argc == 2) ? argv[1] : "./vv-input.json");
  long idCounter = 0;
  VnV_readFile("./vv-output.json", &idCounter);
  INJECTION_FINALIZE()
}
