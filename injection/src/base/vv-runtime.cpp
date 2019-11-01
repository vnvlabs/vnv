
/** @file vv-runtime.cpp **/
#include "vv-runtime.h"

#ifdef WITH_MPI
#  include <mpi.h>
#endif

#include <unistd.h>

#include "VnV.h"
#include "vv-injection.h"
#include "vv-logging.h"
#include "vv-output.h"
#include "vv-parser.h"
#include "vv-unit-tester.h"
#include "vv-utils.h"

using namespace VnV;

void RunTime::injectionPoint(int injectionIndex, std::string scope,
                             std::string function, ...) {
  va_list argp;
  va_start(argp, function);
  injectionPoint(injectionIndex, scope, function, argp);
  va_end(argp);
}

void RunTime::injectionPoint(int injectionIndex, std::string scope,
                             std::string function, va_list argp) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getInjectionPoint(
            scope, injectionIndex);
    if (ipd != nullptr) {
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

void RunTime::loadInjectionPoints(std::string json) {
  JsonParser parser;
  RunInfo info = parser.parse(json);

  // Load the test libraries ( plugins -- This could include a custom engine )
  for (auto it : info.testLibraries) {
    TestStore::getTestStore().addTestLibrary(it);
  }

  // Load the injection Points and create all the tests.
  InjectionPointStore::getInjectionPointStore().addInjectionPoints(
      info.injectionPoints);
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

  JsonParser
      parser;  // =
               // ParserStore::getParserStore().getParser(VnV::getFileExtension(configFile));

  RunInfo info = parser.parse(configFile);

  VnV_Debug("Finished Parsing Json File {}", 10);

  runTests = info.runTests;
  if (runTests) {
    // Load the test libraries ( plugins -- This could include a custom engine )
    for (auto it : info.testLibraries) {
      TestStore::getTestStore().addTestLibrary(it);
    }

    // Load the Output Engine ( tests declare variables when loaded) .
    EngineStore::getEngineStore().setEngineManager(
        info.engineInfo.engineType, info.engineInfo.engineConfig);

    // Load the injection Points and create all the tests.
    InjectionPointStore::getInjectionPointStore().addInjectionPoints(
        info.injectionPoints);

    // Run the initialize Injection point.
    INJECTION_POINT(initialization, -1, int*, argc, char***, argv, std::string,
                    configFile);

  } else if (info.error) {
    VnV_Error("{}", info.errorMessage);
    runTests = false;
  }

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

void RunTime::runUnitTests() {
  UnitTestStore::getUnitTestStore().runAll(false);
}

void VnV_injectionPoint(int stageVal, const char* id, const char* function,
                        ...) {
  va_list argp;
  va_start(argp, function);
  RunTime::instance().injectionPoint(stageVal, id, function, argp);
  va_end(argp);
}

int VnV_init(int* argc, char*** argv, const char* filename) {
  RunTime::instance().Init(argc, argv, filename);
  return 1;
}

int VnV_finalize() {
  RunTime::instance().Finalize();
  return 1;
}

int VnV_runUnitTests() {
  RunTime::instance().runUnitTests();
  return 0;
}
