
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
  } catch (...) {
    std::cout << "Error Initializing VnV" << std::endl;
    std::abort();
  }
}

void VnV_Register_Subpackage(const char* subPackageName, registrationCallBack callback) {
  try {
    VnV::RunTime::instance().runTimePackageRegistration(subPackageName, callback);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Declaring Sub Package");
  }
}

void VnV_declarePackageJson(const char* packageName, vnvFullJsonStrCallback callback) {
  try {
    VnV::RunTime::instance().declarePackageJson(packageName, callback);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Declaring Package");
  }
}

void VnV_finalize() {
  try {
    VnV::RunTime::instance().Finalize();
    VnV::RunTime::reset();
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error During finalization");
  }
}

void VnV_runUnitTests(VnV_Comm comm) {
  try {
    VnV::RunTime::instance().runUnitTests(comm, VnV::UnitTestInfo());
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Unit Tests");
  }
}

void VnV_Registration_Info(const char* filename, int quit) {
  try {
    VnV::RunTime::instance().writeSpecification(filename, quit);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error fettching specification");
  }
}

void VnV_readFile(const char* reader, const char* filename) {
  try {
    VnV::RunTime::instance().readFile(reader, filename);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Reading file %s with reader %s", filename, reader);
  }
}

void VnV_readFileAndWalk(const char* reader, const char* filename, const char* package, const char* walker,
                         const char* config) {
  try {
    VnV::RunTime::instance().readFileAndWalk(reader, filename, package, walker, json::parse(config));

  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error File Read");
  }
}

void VnV_generate_pipeline(const char* package, const char* name, const char* config, const char* filename, int stdo) {
  try {
    VnV::RunTime::instance().pipeline(package, name, json::parse(config), filename, stdo);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error During finalization");
  }
}

void VnV_generate_pipeline_file(const char* package, const char* name, const char* config, const char* filename) {
  try {
    VnV::RunTime::instance().pipeline(package, name, json::parse(config), filename, false);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error generating pipeline");
  }
}

void VnV_generate_pipeline_stdout(const char* package, const char* name, const char* config) {
  try {
    VnV::RunTime::instance().pipeline(package, name, json::parse(config), true);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error generating pipeline");
  }
}

char* VnV_generate_pipeline_string(const char* package, const char* name, const char* config) {
  try {
    VnV_Info(VNVPACKAGENAME, "You need to free this youself!");
    std::string s = VnV::RunTime::instance().pipeline(package, name, json::parse(config), "", false);
    return strdup(s.c_str());
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Generating pipeline");
    return strdup("");
  }
}
}
