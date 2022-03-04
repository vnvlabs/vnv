
/**
  @file Runtime.cpp
**/
#include "base/Runtime.h"

#include <stdarg.h>

#include <stdexcept>

#include "base/stores/CommunicationStore.h"
#include "base/stores/OptionsParserStore.h"

extern "C" {

int VnV_init(const char* packageName, int* argc, char*** argv, const char* filename, registrationCallBack callback) {
  try {
    return VnV::RunTime::instance().InitFromFile(packageName, argc, argv, filename, callback);
  } catch (std::exception& e ) {
    std::cout << "Error Initializing VnV: " << e.what() <<  std::endl;
    std::abort();
  }
}

int VnV_init_raw(const char* packageName, int* argc, char*** argv, const char* inputjson, registrationCallBack callback) {
  try {
    json j = json::parse(inputjson);
    return VnV::RunTime::instance().InitFromJson(packageName, argc, argv, j, callback);
  } catch (std::exception &e) {
    std::cout << "Error Initializing VnV: " << e.what() << std::endl;
    std::abort();
  }
}


void VnV_Register_Subpackage(const char* subPackageName, registrationCallBack callback) {
  try {
    VnV::RunTime::instance().runTimePackageRegistration(subPackageName, callback);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error Declaring Sub Package");
  }
}

void VnV_declarePackageJson(const char* packageName, vnvFullJsonStrCallback callback) {
  try {
    VnV::RunTime::instance().declarePackageJson(packageName, callback);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error Declaring Package");
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

void VnV_runUnitTests(VnV_Comm comm) {
  try {
    VnV::RunTime::instance().runUnitTests(comm, VnV::UnitTestInfo());
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error Running Unit Tests");
  }
}

void VnV_Registration_Info(const char* filename, int quit) {
  try {
    VnV::RunTime::instance().writeSpecification(filename, quit);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error fettching specification");
  }
}

void VnV_readFile(const char* reader, const char* filename) {
  try {
    VnV::RunTime::instance().readFile(reader, filename);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error Reading file %s with reader %s", filename, reader);
  }
}

void* VnV_getOptionsObject(const char* package) {
  return VnV::RunTime::instance().getOptionsObject(package);
}


void VnV_readFileAndWalk(const char* reader, const char* filename, const char* package, const char* walker,
                         const char* config) {
  try {
    VnV::RunTime::instance().readFileAndWalk(reader, filename, package, walker, json::parse(config));

  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Error File Read");
  }
}

}
