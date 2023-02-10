
/**
  @file Logging.cpp
**/
#include "common-interfaces/Logging.h"

#include "base/Runtime.h"

void _VnV_registerLogLevel(const char* packageName, const char* name, const char* color) {
  try {
    VnV::RunTime::instance().registerLogLevel(packageName, name, color);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Error registering log level: %s", e.what());
  }
}

void _VnV_registerFile(VnV_Comm comm, const char* packageName, const char* name, int input, const char* filename,
                       const char* reader) {
  try {
    VnV::RunTime::instance().registerFile(comm, packageName, name, input, filename, reader);

  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Error registering log level: %s", e.what());
  }
}

void _VnV_Log(VnV_Comm comm, const char* p, const char* l, const char* format, ...) {
  try {
    va_list args;
    va_start(args, format);
    VnV::RunTime::instance().log(comm, p, l, format, args);
    va_end(args);
  } catch (std::exception& e) {
    std::cout << "Error during logging: " << e.what() << std::endl;
  }
}
