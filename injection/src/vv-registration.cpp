
//Core VnV Registration functions.

#include "VnV.h"

#ifdef WITH_ADIOS
  #include "plugins/engines/vv-adios.h"
#endif

#include "plugins/engines/vv-debug-engine.h"

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
        VnV::registerEngine("adios", VnV::AdiosEngineBuilder);
#endif
        VnV::registerEngine("debug",VnV::DebugEngineBuilder);

        //Register the tests.
        VnV::registerTest("provenance", ProvenanceTest::maker, ProvenanceTest::declare );

        //Register the injection points hard coded into the code.
        Register_Injection_Point("initialization", initializationConfig);
    }
  }
}

