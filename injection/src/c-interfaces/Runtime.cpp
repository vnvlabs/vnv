
/**
  @file Runtime.cpp
**/
#include "base/Runtime.h"

#include <stdarg.h>

#include <stdexcept>

#include "base/stores/CommunicationStore.h"
#include "base/stores/OptionsParserStore.h"

extern "C" {

int VnV_init(const char* packageName, int* argc, char*** argv,
             const char* filename, registrationCallBack callback) {
  return VnV::RunTime::instance().InitFromFile(packageName, argc, argv,
                                               filename, callback);
}

void VnV_Register_Subpackage(const char* subPackageName,
                             registrationCallBack callback) {
  VnV::RunTime::instance().runTimePackageRegistration(subPackageName, callback);
}

void VnV_declarePackageJson(const char* packageName,
                            vnvFullJsonStrCallback callback) {
  VnV::RunTime::instance().declarePackageJson(packageName, callback);
}

void VnV_finalize() {
  VnV::RunTime::instance().Finalize();
  VnV::RunTime::reset();
}

void VnV_runUnitTests(VnV_Comm comm) {
  VnV::RunTime::instance().runUnitTests(comm, VnV::UnitTestInfo());
}

void VnV_Registration_Info(const char* filename, int quit) {
  VnV::RunTime::instance().writeSpecification(filename, quit);
}

void VnV_readFile(const char* reader, const char* filename) {
  VnV::RunTime::instance().readFile(reader, filename);
}

void VnV_readFileAndWalk(const char* reader, const char* filename,
                         const char* package, const char* walker,
                         const char* config) {
  VnV::RunTime::instance().readFileAndWalk(reader, filename, package, walker,
                                           json::parse(config));
}

void VnV_generate_pipeline(const char* package, const char* name, const char* config, const char* filename, int stdo) {
  VnV::RunTime::instance().pipeline(package,name,json::parse(config),filename, stdo);
}

void VnV_generate_pipeline_file(const char* package, const char* name, const char* config, const char* filename) {
  VnV::RunTime::instance().pipeline(package,name,json::parse(config),filename, false);
}

void VnV_generate_pipeline_stdout(const char* package, const char* name, const char* config) {
  VnV::RunTime::instance().pipeline(package,name,json::parse(config),true);
}

char* VnV_generate_pipeline_string(const char* package, const char* name, const char* config) {
   VnV_Info(VNVPACKAGENAME, "You need to free this youself!");
   std::string s = VnV::RunTime::instance().pipeline(package,name,json::parse(config),"",false);
   return strdup(s.c_str());
}




VNVEXTERNC void VnV_generate_pipeline_stdout(const char* package, const char* name, const char* config);


}
