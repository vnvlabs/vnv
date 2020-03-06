
/**
  @file Registration.cpp
**/
//Core VnV Registration functions.

#include "Registration.h"

#include "interfaces/ITest.h"
#include "c-interfaces/CppInjection.h"

#ifdef WITH_ADIOS
  #include "plugins/engines/AdiosOutputEngineImpl.h"
#endif

#include "plugins/engines/DebugOutputEngineImpl.h"

namespace VnV {
 namespace {
#ifdef WITH_ADIOS
    OutputEngineManager* AdiosEngineBuilder() { return new AdiosWrapper(); }
#endif
    OutputEngineManager* DebugEngineBuilder() { return new DebugEngineWrapper();}
 }
}

namespace ProvenanceTest {
    VnV::ITest* maker(VnV::TestConfig config);
    json declare();
};


static const std::string initializationConfig = R"(
{
   "name" : "initialization",
   "parameters" : {
        "argc" : "int*" ,
        "argv" : "char***" ,
        "configFile" : "std::string"
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
    void registerVnV() {

        // Register the engines.
#ifdef WITH_ADIOS
        VnV::registerEngine("adios", VnV::AdiosEngineBuilder);
#endif
        VnV::registerEngine("debug",VnV::DebugEngineBuilder);

        //Register the tests.
        VnV::registerTest("provenance", ProvenanceTest::maker, ProvenanceTest::declare );

        //Register the injection points hard coded into the code.
        Register_Injection_Point(initializationConfig);
    }
  }
}

