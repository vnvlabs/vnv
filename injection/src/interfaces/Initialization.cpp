
/**
  @file Runtime.cpp
**/
#include "interfaces/Initialization.h"

#include "base/Runtime.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {

void defaultInitCallback(VnV_Comm comm, VnV::IOutputEngine* engine){};

int Init(const char* packageName, int* argc, char*** argv, InitDataCallback icallback,
         registrationCallBack callback) {
  try {
    return VnV::RunTime::instance().InitFromFile(packageName, argc, argv, icallback, callback);
  } catch (std::exception& e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
  }
  return 0;
}

void finalize() {
  try {
    VnV::RunTime::instance().Finalize();
    VnV::RunTime::reset();
  } catch (std::exception& e) {
    std::cout << "Error Finalizing VnV: " << e.what() << std::endl;
  }
}

}  // namespace VnV
