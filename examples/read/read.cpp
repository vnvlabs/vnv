
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
  
  json j = json::object();
  j["collection"] = VnV::StringUtils::random(4);
  j["persist"] = "memory";
  auto a = VnV::Python::ReaderWrapper(argv[2], argv[1], j.dump(), false);
  
  INJECTION_FINALIZE(RPNAME)
}
