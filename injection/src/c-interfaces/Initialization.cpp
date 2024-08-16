
/**
  @file Runtime.cpp
**/
#include "interfaces/Initialization.h"

#include <stdarg.h>

#include <stdexcept>

#include "base/Runtime.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/OptionsParserStore.h"
#include "c-interfaces/Wrappers.h"

namespace {

VnV::InitDataCallback cpp_to_c_wrapper(initDataCallback callback) {
  return [callback](VnV_Comm comm, VnV::IOutputEngine* engine) {
    if (callback) {
      IOutputEngineWrapper wrapper = {engine};
      callback(comm, &wrapper);
    }
  };
}

}  // namespace

extern "C" {

int VnV_init(const char* packageName, int* argc, char*** argv, initDataCallback icallback, registrationCallBack callback) {
  try {
    return VnV::RunTime::instance().InitFromFile(packageName, argc, argv, cpp_to_c_wrapper(icallback), callback);
  } catch (std::exception& e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
  }
  return 0;
}


void VnV_finalize() {
  try {
    VnV::RunTime::instance().Finalize();
    VnV::RunTime::reset();
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Error During finalization");
  }
}
}
