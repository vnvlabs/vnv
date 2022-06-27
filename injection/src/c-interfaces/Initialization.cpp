
/**
  @file Runtime.cpp
**/
#include "base/Runtime.h"

#include <stdarg.h>

#include <stdexcept>

#include "base/stores/CommunicationStore.h"
#include "base/stores/OptionsParserStore.h"
#include "c-interfaces/Wrappers.h"

#include "interfaces/Initialization.h"

namespace {

VnV::InitDataCallback cpp_to_c_wrapper(initDataCallback callback) {
  
  return [callback](VnV_Comm comm, VnV::IOutputEngine *engine) {
    if (callback) {
      IOutputEngineWrapper wrapper = {engine};
      callback(comm,&wrapper);
    }
  };
}

}


extern "C" {


int VnV_init(const char* packageName, int* argc, char*** argv, const char* filename, initDataCallback icallback, registrationCallBack callback) {
  try {

    return VnV::RunTime::instance().InitFromFile(packageName, argc, argv, filename, cpp_to_c_wrapper(icallback), callback);
  } catch (std::exception& e ) {
    std::cout << "Error Initializing VnV: " << e.what() <<  std::endl;
    std::abort();
  }
}

int VnV_init_raw(const char* packageName, int* argc, char*** argv, const char* inputjson, initDataCallback icallback, registrationCallBack callback) {
  try {
    json j = json::parse(inputjson);
    return VnV::RunTime::instance().InitFromJson(packageName, argc, argv, j, cpp_to_c_wrapper(icallback), callback);
  } catch (std::exception &e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
    std::abort();
  }
}

void VnV_finalize() {
  try {
    VnV::RunTime::instance().Finalize();
    VnV::RunTime::reset();
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error During finalization");
  }
}

}


