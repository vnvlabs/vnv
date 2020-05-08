
/**
  @file Registration.cpp
**/
//Core VnV Registration functions.
#include <string>
#include "base/InjectionPointStore.h"
#include "Registration.h"

static const std::string initializationConfig = R"(
{
   "name" : "initialization",
   "package" : "VnV",
   "parameters" : {
        "argc" : "int*" ,
        "argv" : "char***" ,
        "config" : "json"
   },
   "documentation" : {
      "type" : "SINGLE",
      "package" : "VnV",
      "function" : "RunTime::Init",
      "description" : "Initial Injection point containing the command line parameters and config file name"
   }
}
)";

namespace VnV {
  namespace Registration {
    void RegisterBuiltinEngines();
    void RegisterBuiltinTests();
    void RegisterBuiltinUnitTests();
    
    void registerVnV() {
         //Register the engines
         RegisterBuiltinEngines();
         RegisterBuiltinTests();
         RegisterBuiltinUnitTests();
 
         //Register the injection points.
         VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(initializationConfig);
    }
  }
}

