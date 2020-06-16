
/**
  @file Runtime.cpp
**/
#include "base/Runtime.h"

#include <stdarg.h>

#include <stdexcept>

#include "base/CommunicationStore.h"
#include "base/OptionsParserStore.h"

extern "C" {

void VnV_init(const char* packageName, int* argc, char*** argv,
              const char* filename, registrationCallBack callback) {
  VnV::RunTime::instance().InitFromFile(packageName, argc, argv, filename,
                                        &callback);
}

void VnV_Register_Subpackage(const char* packageName,
                             const char* subPackageName,
                             registrationCallBack callback) {
  VnV::RunTime::instance().runTimePackageRegistration(subPackageName, callback);
}

void VnV_declarePackageJson(const char* packageName,
                            vnvFullJsonStrCallback callback) {
  VnV::RunTime::instance().declarePackageJson(packageName, callback);
}

void VnV_finalize() { VnV::RunTime::instance().Finalize(); }

void VnV_runUnitTests(VnV_Comm comm) {
  VnV::RunTime::instance().runUnitTests(comm);
}

void VnV_Registration_Info(const char* filename) {
    VnV::RunTime::instance().writeSpecification(filename);
}

void VnV_readFile(const char* filename) {
  VnV::RunTime::instance().readFile(filename);
}
}
