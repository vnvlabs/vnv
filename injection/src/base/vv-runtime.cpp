
/** @file vv-runtime.cpp **/
#include "vv-runtime.h"

#ifdef WITH_MPI
#  include <mpi.h>
#endif

#if defined(__APPLE__)
#include <link.h>
#endif

#include <unistd.h>

#include "VnV.h"
#include "vv-injection.h"
#include "vv-logging.h"
#include "vv-output.h"
#include "vv-parser.h"
#include "vv-unit-tester.h"
#include "vv-utils.h"
#include "vv-dist-utils.h"

using namespace VnV;

/**
 * Anon Namespace to house ld iterate callback code used in fetchInjectionPoint information. (only accesible in this file).
 */
namespace {


 int callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {
    json &j = *(static_cast<json*>(data));

    std::string name(info->dlpi_name);  // Library name
    try {
        if ( j.find(name) == j.end()) {
            // If we have not seen this.
            void* dllib = dlopen(name.c_str(), RTLD_NOW);
            if (dllib == nullptr)
                std::cerr << dlerror();
            else {
                 // Search the shared library for the symbol named: INJECTION_POINT_DECLARE.
                 // This assumes that the linked library exports a symbol called __vnv__registration__
                 // that contains functions injectionPoints(), tests(), unitTests(), etc,
                struct VnV_Registration * vv_info;
                vv_info = static_cast<VnV_Registration*>(dlsym(dllib, VNV_REG_STR));
                if ( vv_info != nullptr ) {
                    json libJson = json::parse(vv_info->info());
                    j[name] = json::parse(vv_info->info());
                }
            }
        }
     } catch (...) {
        VnV_Error("Could not load Shared Library %s", name.c_str());
     }
     return 0;
 }

 json iterateLinkedLibraries() {
    json info;
    DistUtils::iterateLinkedLibraries(callback, static_cast<void*>(&info));
    return info;
 }

}

json RunTime::fetchInjectionPointInformation() {
    return iterateLinkedLibraries();
}


void RunTime::injectionPoint(std::string pname, int injectionIndex, std::string scope,
                             std::string function, ...) {
  va_list argp;
  va_start(argp, function);
  injectionPoint(pname, injectionIndex, scope, function, argp);
  va_end(argp);
}

void RunTime::injectionPoint(std::string pname, int injectionIndex, std::string scope,
                             std::string function, va_list argp) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getInjectionPoint(
            scope, injectionIndex);
    if (ipd != nullptr)
    {
      VnV_Info("Running tests at injection Point %s from package %s inside function %s", scope.c_str(),pname.c_str(),function.c_str());
      ipd->runTests(injectionIndex, argp);
    } else {
    }
  }
}

RunTime& RunTime::instance() {
  static RunTime* store = new RunTime();
  return *store;
}

RunTime::RunTime() {}

void RunTime::loadRunInfo(RunInfo &info) {

  if (info.logInfo.on) {
       logger.setLog(info.logInfo.filename);
     for (auto it: info.logInfo.logs) {
         logger.setLogLevel(it.first,it.second);
     }
     for ( auto it : info.logInfo.blackList ) {
        logger.addToBlackList(it);
     }
  }

  // Load the test libraries ( plugins -- This could include a custom engine )
  //VnV_Debug("Loading Test Libraries");
  for (auto it : info.testLibraries) {
    TestStore::getTestStore().addTestLibrary(it);
    //VnV_Debug("\t Loaded Test Library {}" , it);
  }


  if (!EngineStore::getEngineStore().isInitialized()) {
      //VnV_Debug("Configuring The Output Engine");
      EngineStore::getEngineStore().setEngineManager(
            info.engineInfo.engineType, info.engineInfo.engineConfig);
      //VnV_Debug("Output Engine Configuration Successful");
  }

  std::map<std::string, std::vector<TestConfig>> configs;
  //VnV_Debug("Validating Json Test Configuration Input and converting to TestConfig objects");
  for ( auto it : info.injectionPoints) {
      configs.insert(std::make_pair(it.first, TestStore::getTestStore().validateTests(it.second))) ;
  }
  // Load the injection Points and create all the tests.
  //VnV_Debug("Loading Injection Points into Injection Point Store");
  InjectionPointStore::getInjectionPointStore().addInjectionPoints(configs);

}


void RunTime::loadInjectionPoints(json _json) {
  JsonParser parser;
  RunInfo info = parser.parse(_json);
  loadRunInfo(info);
}

bool RunTime::Init(int* argc, char*** argv, std::string configFile) {
  int flag = 0;

#ifdef WITH_MPI
  MPI_Initialized(&flag);
  if (!flag) {
    MPI_Init(argc, argv);

    // Note that we initialized MPI, so, we should finalize it as well.
    finalize_mpi = true;
  }
#endif

  JsonParser parser;
  std::ifstream in(configFile);
  RunInfo info = parser.parse(in);

  runTests = info.runTests;
  if (runTests) {
    loadRunInfo(info);
    INJECTION_POINT(initialization, -1, int*, argc, char***, argv, std::string,configFile);
  } else if (info.error) {
    runTests = false;
  }

  printRunTimeInformation();

  return runTests;
}

bool RunTime::Finalize() {
  if (runTests) {
    EngineStore::getEngineStore().getEngineManager()->finalize();
#ifdef WITH_MPI
    if (finalize_mpi) {
      int flag = 0;
      MPI_Finalized(&flag);
      if (!flag) {
        MPI_Finalize();
      }
    }
#endif
    return true;
  }
  return true;
}

bool RunTime::isRunTests() { return runTests; }


void RunTime::log(std::string pname, LogLevel level, std::string message, va_list args) {
    logger.log_c(pname, level, message, args);
}

void RunTime::runUnitTests() {
  UnitTestStore::getUnitTestStore().runAll(false);
}


void RunTime::printRunTimeInformation() {
        VnV_BeginStage("Printing The RunTime Configuration.");
        logger.print();
        EngineStore::getEngineStore().print();
        TestStore::getTestStore().print();
        InjectionPointStore::getInjectionPointStore().print();
        VnV_EndStage("");
}






