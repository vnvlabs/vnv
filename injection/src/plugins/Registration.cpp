
/**
  @file Registration.cpp
**/
//Core VnV Registration functions.
#include <string>
#include "VnV.h"

namespace VnV {
  namespace Registration {
    void RegisterBuiltinEngines();
    void RegisterBuiltinTests();
    void RegisterBuiltinUnitTests();
    void RegisterBuiltinSerializers();
    void RegisterBuiltinTransforms();
    void RegisterBuiltinInjectionPoints() {
         Register_Injection_Point("initialization",R"({"argc":"int*","argv":"char***","config":"json"})");
    }
  }
}

/**
 * Define the actual Registration callback for the VNV library plugins.
 */
INJECTION_REGISTRATION(){
     VnV::Registration::RegisterBuiltinEngines();
     VnV::Registration::RegisterBuiltinTests();
     VnV::Registration::RegisterBuiltinUnitTests();
     VnV::Registration::RegisterBuiltinTransforms();
     VnV::Registration::RegisterBuiltinSerializers();
     VnV::Registration::RegisterBuiltinInjectionPoints();
}



