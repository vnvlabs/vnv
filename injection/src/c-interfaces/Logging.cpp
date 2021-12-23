
/**
  @file Logging.cpp
**/
#include "c-interfaces/Logging.h"

#include "base/Runtime.h"

void _VnV_registerLogLevel(const char* packageName, const char* name,
                           const char* color) {
  try {
    VnV::RunTime::instance().registerLogLevel(packageName, name, color);
  } catch(...) {
    VnV_Error(VNVPACKAGENAME, "Error registering log level");
  }
}

void _VnV_registerFile(VnV_Comm comm, const char* packageName, const char* name,
                       int input, const char* filename, const char* reader) {
try {
  
  VnV::RunTime::instance().registerFile(comm, packageName, name, input, filename, reader);

} catch(...) {
    VnV_Error(VNVPACKAGENAME, "Error registering log level");
}

}

void _VnV_Log(VnV_Comm comm, const char* p, const char* l, const char* format,
              ...) {
  try {
    va_list args;
    va_start(args, format);
    VnV::RunTime::instance().log(comm, p, l, format, args);
    va_end(args);
  } catch(...) {
    std::cout << "Error during logging " << std::endl;
  }
}
