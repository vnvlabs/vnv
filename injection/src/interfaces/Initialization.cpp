
/**
  @file Runtime.cpp
**/
#include "interfaces/Initialization.h"

#include "base/Runtime.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {

void defaultInitCallback(VnV_Comm comm, VnV::IOutputEngine* engine){};

int Init(const char* packageName, int* argc, char*** argv, const char* filename, InitDataCallback icallback,
         registrationCallBack callback) {
  try {
    return VnV::RunTime::instance().InitFromFile(packageName, argc, argv, filename, icallback, callback);
  } catch (std::exception& e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
  }
}

int Init_raw(const char* packageName, int* argc, char*** argv, const char* inputjson, InitDataCallback icallback,
             registrationCallBack callback) {
  try {
    json j = json::parse(inputjson);
    return VnV::RunTime::instance().InitFromJson(packageName, argc, argv, j, icallback, callback);
  } catch (std::exception& e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
  }
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
