
//Core VnV Registration functions.

#include "VnV.h"
#include "VnV-Interfaces.h"

#ifdef WITH_ADIOS
  #include "vv-adios.h"
#endif

#include "vv-debug-engine.h"

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
namespace StdRerouteTest{
    VnV::ITest* maker(VnV::TestConfig config);
    json declare();

}

static const char * initializationConfig = R"(
{
   "name" : "initialization",
   "type" : "SINGLE",
   "package" : "VnV",
   "function" : "RunTime::Init",
   "description" : "Initial Injection point containing the command line parameters and config file name",
   "restructuredText" : "VnV Initialization is Complete",
   "stages" : {
       "BEGIN" : {
          "parameters" : {
                "argc" : {
                    "class" : "int*",
                    "description" : "argc from the command line"
                },
                "argv" : {
                    "class" : "char***" ,
                    "description" : "argv from the command line"
                },
                "configFile" : {
                  "class" : "std::string",
                  "description" : "the filename of the configuration file used to configure the runtime"
                }
          }
       }
   }
})";

namespace VnV {
  namespace Registration {
    void registerVnV() {

        // Register the engines.
#ifdef WITH_ADIOS
        VnV_registerEngine("adios", VnV::AdiosEngineBuilder);
#endif
        VnV_registerEngine("debug",VnV::DebugEngineBuilder);

        //Register the tests.
        VnV_registerTest("provenance", ProvenanceTest::maker, ProvenanceTest::declare );
        VnV_registerTest("std-reroute", StdRerouteTest::maker, StdRerouteTest::declare );

        //Register the injection points hard coded into the code.
        Register_Injection_Point("initialization", initializationConfig);

        //Register a custom log level for VnV
        Register_Log_Level("custom", "\033[36m")
        VnV_Log("custom", "Custom log level registered with cyan output");
    }
  }
}

