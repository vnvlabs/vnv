
#include <iostream>
#include <vector>
#define RPNAME SampleReader
#include "VnV.h"
#include "python/PythonInterface.h"
INJECTION_EXECUTABLE(RPNAME)

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: ./read.a <engine-reader> <filename>";
    return 1;
  }

  

  INJECTION_INITIALIZE(RPNAME, &argc, &argv, VNV_DEFAULT_INPUT_FILE);
  
  auto a = VnV::Python::ReaderWrapper(argv[2], argv[1], "{}", false);
  
  INJECTION_FINALIZE(RPNAME)
}
