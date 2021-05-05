
#include <iostream>
#include <vector>
#define RPNAME SampleReader
#include "VnV.h"

INJECTION_EXECUTABLE(RPNAME, VNV, serial)

int main(int argc, char** argv) {

  INJECTION_INITIALIZE(RPNAME, &argc, &argv,
                       (argc == 2) ? argv[1] : "./vv-input.json");
  long idCounter = 0;
  VnV_readFile("./vv-output.json", &idCounter);
  INJECTION_FINALIZE(RPNAME)
}
