#ifndef VV_RUNTIME_HEADER
#define VV_RUNTIME_HEADER

#include <string>
#include <stdarg.h>

namespace VnV {

class RunTime {

private:
  RunTime();
  bool runTests;

  public:
 
    bool Init(std::string configFile);
    void injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) ; 
    void injectionPoint(int injectionIndex, std::string scope, std::string function, ...) ;
    bool Finalize();
    bool isRunTests();
    static RunTime& instance(); 
  };
}


extern "C" {
  void VnV_injectionPoint(int stageVal, const char * id, const char * function, ...); 
  int VnV_init(const char *filename);
  int VnV_finalize();
}

#endif

