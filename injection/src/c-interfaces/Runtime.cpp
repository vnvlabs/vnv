
/**
  @file Runtime.cpp
**/
#include <stdarg.h>
#include <stdexcept>
#include "base/Runtime.h"
#include "base/OptionsParserStore.h"

extern "C" {

void  VnV_init(int* argc, char*** argv, const char* filename, registrationCallBack callback) {
  VnV::RunTime::instance().InitFromFile(argc, argv, filename,&callback);
}

void VnV_finalize() {
  VnV::RunTime::instance().Finalize();
}

void VnV_runUnitTests(VnV_Comm comm ) {
  VnV::RunTime::instance().runUnitTests(comm);
}

void VnV_readFile(const char* filename) {
    VnV::RunTime::instance().readFile(filename);
}

}
