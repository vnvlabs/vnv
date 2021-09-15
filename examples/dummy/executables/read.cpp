
#include <iostream>
#include <vector>
#define RPNAME SampleReader
#include "VnV.h"

INJECTION_EXECUTABLE(RPNAME)

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: ./read.a <engine-reader> <filename>";
    return 1;
  }

  INJECTION_INITIALIZE(RPNAME, &argc, &argv, VNV_DEFAULT_INPUT_FILE);
  long idCounter = 0;
  VnV_readFileAndWalk(argv[1], argv[2], "VNV", "proc",
                      R"({"id":0,"only":false,"comm":false})");

  INJECTION_FINALIZE(RPNAME)
}
