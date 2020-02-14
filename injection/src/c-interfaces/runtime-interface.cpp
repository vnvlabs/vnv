
#include <stdarg.h>
#include <stdexcept>
#include "base/vv-runtime.h"
#include "base/vv-options-parser.h"

extern "C" {

void  VnV_init(int* argc, char*** argv, const char* filename, registrationCallBack callback) {
  VnV::RunTime::instance().Init(argc, argv, filename,&callback);
}

void VnV_finalize() {
  VnV::RunTime::instance().Finalize();
}

void VnV_runUnitTests() {
  VnV::RunTime::instance().runUnitTests();
}

void _VnV_registerOptions(const char* name, options_schema_ptr s, options_callback_ptr v) {
   VnV::OptionsParserStore::instance().add(name,s,v);
}


}
