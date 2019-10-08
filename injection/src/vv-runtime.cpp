
#include "vv-runtime.h"
#include "vv-injection.h"
#include "vv-parser.h"
#include "vv-output.h"
#include "vv-utils.h"

using namespace VnV;

void RunTime::injectionPoint(int injectionIndex, std::string scope, std::string function, ...) {
    va_list argp;
    va_start(argp, function);
    injectionPoint(injectionIndex, scope, function, argp);
    va_end(argp);
}

void RunTime::injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) {

    if ( runTests ) {
       InjectionPoint *ipd =  InjectionPointStore::getInjectionPointStore().getInjectionPoint(scope);
        if ( ipd != NULL ) {
          ipd->runTests(injectionIndex, argp );
        } else {
        }
    }
}

RunTime& RunTime::instance() {
      static RunTime * store = new RunTime();
      return *store;
}

RunTime::RunTime(){}

bool RunTime::Init(std::string configFile) {
    
    IParser *parser = ParserStore::getParserStore().getParser(VnV::getFileExtension(configFile));
    
    RunInfo info =  parser->parse(configFile);
    runTests = info.runTests;
    if ( runTests ) {
      
      // Load the test libraries ( plugins -- This could include a custom engine )  
      for ( auto it : info.testLibraries ) {
        TestStore::getTestStore().addTestLibrary(it);
      }
      
      // Load the Output Engine ( tests declare variables when loaded) . 
      EngineStore::getEngineStore().setEngineManager(info.engineInfo.engineType, 
                info.engineInfo.outFileName, 
                info.engineInfo.engineConfig, 
                info.engineInfo.debug
      );

    
      // Load the injection Points and create all the tests. 
      for ( auto it : info.injectionPoints) {
        InjectionPoint *ip = new InjectionPoint(it.first);
        for ( auto testConfig : it.second ) {  
          // Add the test. 
          ip->addTest(testConfig);
        }
        InjectionPointStore::getInjectionPointStore().addInjectionPoint(it.first, ip);
      }
    } else if (info.error ) {
        std::cout << info.errorMessage << std::endl;
        runTests = false;
    }
      

    delete parser;
    return runTests;

}

bool RunTime::Finalize() {
  if ( runTests ) 
    EngineStore::getEngineStore().getEngineManager()->finalize();
}

bool RunTime::isRunTests() {
  return runTests;
}

void VnV_injectionPoint(int stageVal, const char * id, const char * function, ...) {
    va_list argp;
    va_start(argp, function);
    RunTime::instance().injectionPoint(stageVal, id, function, argp);
    va_end(argp);
}

int VnV_init(const char * filename) {
  RunTime::instance().Init(filename);
  return 1;
}

int VnV_finalize() {
  RunTime::instance().Finalize();
  return 1;
}











