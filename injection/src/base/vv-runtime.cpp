
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
#include "vv-options-parser.h"
#include "vv-registration.h"

using namespace VnV;



/**
 * Anon Namespace to house ld iterate callback code used in fetchInjectionPoint information. (only accesible in this file).
 */
namespace {


 bool loadLibrary(std::string name) {
    if ( name.empty()) return false;
    void* dllib = dlopen(name.c_str(), RTLD_NOW);

    if (dllib == nullptr) {
        throw "Could not open shared library";
    }
    bool ret = false;
    auto a = VnV_BeginStage("Searching %s for VnV Registration Callback", name.c_str());
    void* callback = dlsym(dllib,VNV_REGISTRATION_CALLBACK_NAME_STR);
    if ( callback != nullptr ) {
       VnV_Debug("Found A Registration CallBack. Calling it %s", name.c_str());
       ((registrationCallBack) callback)();
       dlclose(dllib);
       ret = true;
    } else {
       VnV_Debug("Library %s did not register a callback", name.c_str());
       dlclose(dllib);
       ret = false;
    }
    VnV_EndStage(a);
    return ret;
 }

 int callback(struct dl_phdr_info* info, size_t /*size*/, void* /*data*/) {
    std::string name(info->dlpi_name);  // Library name
    try {
       loadLibrary(name);
     } catch (...) {
        VnV_Error("Could not load Shared Library %s", name.c_str());
     }
     return 0;
 }

}

void RunTime::makeLibraryRegistrationCallbacks() {
   DistUtils::iterateLinkedLibraries(callback, nullptr);
}

bool RunTime::useAsciiColors() {
    return terminalSupportsAsciiColors;
}

void RunTime::logUnhandled(std::string name, std::string id, std::string stageId, std::string function, std::string file, int line, va_list argp) {
       auto aa = VnV_BeginStage("Unhandled InjectionPoint");
       VnV_Info("Package: %s",name.c_str());
       VnV_Info("Name: %s",id.c_str());
       VnV_Info("Stage: %s ", stageId.c_str());
       VnV_Info("File: %s",file.c_str());
       VnV_Info("Function: %s",function.c_str());
       VnV_Info("Line: %d", line);
       auto a = VnV_BeginStage("Available Parameters:");
       NTV ntv;
       InjectionPoint::unpack_parameters(ntv,argp);
       for ( auto &it : ntv ) {
           VnV_Info("%s %s %p",it.first.c_str(),it.second.first.c_str(), it.second.second);
       }
       VnV_EndStage(a);
       VnV_EndStage(aa);
}

void RunTime::_injectionPoint(std::string pname, std::string id, InjectionPointType type, std::string function, std::string file, int line, va_list argp, std::string stageId) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getInjectionPoint(id,type,stageId);
    if (ipd != nullptr)
    {
        ipd->runTests(argp);
    } else {
        logUnhandled(pname,id,stageId,function,file,line,argp);
    }
  }

}

void RunTime::injectionPoint_end(std::string pname, std::string id, std::string function, std::string file, int line, va_list argp) {
    _injectionPoint(pname,id,InjectionPointType::End,function,file,line,argp,"End");
}

void RunTime::injectionPoint_begin(std::string pname, std::string id, std::string function, std::string file, int line, va_list argp) {
    _injectionPoint(pname,id,InjectionPointType::Begin,function,file,line,argp,"Begin");
}

void RunTime::injectionPoint(std::string pname, std::string id, std::string function, std::string file, int line, va_list argp) {
    _injectionPoint(pname,id,InjectionPointType::Single,function,file,line,argp,"Single");
}
void RunTime::injectionPoint_iter(std::string pname, std::string id, std::string stageId, std::string function, std::string file, int line, va_list argp) {
    _injectionPoint(pname,id,InjectionPointType::Iter,function,file,line,argp,stageId);
}

RunTime& RunTime::instance() {
  static RunTime* store = new RunTime();
  return *store;
}

RunTime::RunTime() {}

void RunTime::registerLogLevel(std::string name, std::string color) {
    logger.registerLogLevel(name,color);
}

void RunTime::loadRunInfo(RunInfo &info, registrationCallBack *callback) {

  // Set up the logger.
  if (info.logInfo.on) {
       logger.setLog(info.logInfo.filename);
     for (auto it: info.logInfo.logs) {
         logger.setLogLevel(it.first,it.second);
     }
     for ( auto it : info.logInfo.blackList ) {
        logger.addToBlackList(it);
     }
  }

  //Call this libraries registration callback object (defined in vv-registration.cpp) .
  VnV::Registration::registerVnV();


  // Load the test libraries ( plugins -- This could include a custom engine )
  //VnV_Debug("Loading Test Libraries");
  for (auto it : info.testLibraries) {
    TestStore::getTestStore().addTestLibrary(it);
    //VnV_Debug("\t Loaded Test Library {}" , it);
  }



  //Call All the linked libraries registration callbacks. This allows
  // linked libraries to declare injectionPoints, engines, unit tests, options,
  // etc programatically at runtime. This is not required, but extremely useful
  // for declaring injection points. The better approach would be to declare
  // items using the static initializer approach, but that is not possible
  // is C and fortran. Through this callback, users should call the "VnV_RegisterInjectionPoint
  // " function to declare all injection points. The clang libtooling tool will be designed to
  // automatiically generate this file" .
  makeLibraryRegistrationCallbacks();

  // Call the executables registration callback.
  if (callback != nullptr) {
      (*callback)();
  }

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.toolConfig);


  if (!EngineStore::getEngineStore().isInitialized()) {
      VnV_Debug("Configuring The Output Engine");
      EngineStore::getEngineStore().setEngineManager(
            info.engineInfo.engineType, info.engineInfo.engineConfig);
      VnV_Debug("Output Engine Configuration Successful");
  }

  std::map<std::string, std::vector<TestConfig>> configs;
  VnV_Debug("Validating Json Test Configuration Input and converting to TestConfig objects");
  for ( auto it : info.injectionPoints) {
      configs.insert(std::make_pair(it.first, TestStore::getTestStore().validateTests(it.second))) ;
  }
  // Load the injection Points and create all the tests.
  VnV_Debug("Loading Injection Points into Injection Point Store");
  InjectionPointStore::getInjectionPointStore().addInjectionPoints(configs);

}


void RunTime::loadInjectionPoints(json _json) {
  JsonParser parser;
  RunInfo info = parser.parse(_json);
  loadRunInfo(info,nullptr);
}

bool RunTime::Init(int* argc, char*** argv, std::string configFile, registrationCallBack *callback) {
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
   loadRunInfo(info,callback);
   printRunTimeInformation();
   INJECTION_POINT(initialization, int*, argc, char***, argv, std::string,configFile);
  } else if (info.error) {
    runTests = false;
    processToolConfig(info.toolConfig);
  }


  return runTests;
}


void RunTime::processToolConfig(json config) {
   OptionsParserStore::instance().parse(config);
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


void RunTime::log(std::string pname, std::string level, std::string message, va_list args) {
    logger.log_c(pname, level, message, args);
}

int RunTime::beginStage(std::string pname, std::string message, va_list args) {
 return   logger.beginStage(pname, message, args);
}

void RunTime::endStage(int ref) {
    logger.endStage(ref);
}

void RunTime::runUnitTests() {
  UnitTestStore::getUnitTestStore().runAll(false);
}


void RunTime::printRunTimeInformation() {
        int a = VnV_BeginStage("Runtime Configuration");
        logger.print();
        EngineStore::getEngineStore().print();
        TestStore::getTestStore().print();
        InjectionPointStore::getInjectionPointStore().print();
        VnV_EndStage(a);
}






