
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#ifdef WITH_MPI
#  include <mpi.h>
#endif

#if defined(__APPLE__)
#include <link.h>
#endif

//#include <unistd.h>
#include "base/Runtime.h"
#include "base/TestStore.h"
#include "base/DistUtils.h"
#include "base/InjectionPointStore.h"
#include "base/JsonSchema.h"
#include "base/OutputEngineStore.h"
#include "base/OptionsParserStore.h"
#include "base/UnitTestStore.h"
#include "Registration.h"

#include "c-interfaces/Logging.h"

using namespace VnV;



/**
 * Anon Namespace to house ld iterate callback code used in fetchInjectionPoint information. (only accesible in this file).
 */


void RunTime::makeLibraryRegistrationCallbacks(std::map<std::string, std::string> packageNames) {
   DistUtils::callAllLibraryRegistrationFunctions(packageNames);
}

bool RunTime::useAsciiColors() {
    return terminalSupportsAsciiColors;
}

void RunTime::logUnhandled(std::string name, std::string id, NTV &args) {
    int a = VnV_BeginStage("Unhandled Injection Point");
    VnV_Info("Name: %s", name.c_str());
    VnV_Info("ID: %s", id.c_str());
    int aa = VnV_BeginStage("Parameters");
    for ( auto &it : args) {
       VnV_Info("%s : (%s)", it.first.c_str(),it.second.first.c_str());
    }
    VnV_EndStage(aa);
    VnV_EndStage(a);
}

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(std::string pname, std::string id, InjectionPointType type, NTV &args) {
  if (runTests) {

    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getNewInjectionPoint(id,type,args);
    if (ipd != nullptr) {
        ipd->setInjectionPointType(type,"Begin");
        return ipd;
    } else if (runTimeOptions.logUnhandled) {
        logUnhandled(pname,id,args);
    }
  }
  return nullptr;
}

std::shared_ptr<InjectionPoint> RunTime::getExistingInjectionPoint(std::string pname, std::string id, InjectionPointType type, std::string stageId) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getExistingInjectionPoint( id,type);
    if (ipd != nullptr) {
        ipd->setInjectionPointType(type,stageId);
        return ipd;
    }
  }
  return nullptr;
}


// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, const CppInjection::DataCallback &callback, NTV &args) {
    auto it = getNewInjectionPoint(pname,id,InjectionPointType::Single,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, const CppInjection::DataCallback &callback, NTV &args) {
    auto it  =getNewInjectionPoint(pname,id,InjectionPointType::Begin,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, injectionDataCallback *callback, NTV &args) {
    auto it = getNewInjectionPoint(pname,id,InjectionPointType::Single,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint_begin(VnV_Comm comm,std::string pname, std::string id, injectionDataCallback *callback, NTV &args) {
    auto it  =getNewInjectionPoint(pname,id,InjectionPointType::Begin,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint_iter(std::string pname, std::string id,std::string stageId) {
    auto it = getExistingInjectionPoint(pname,id,InjectionPointType::Iter,stageId);
    if (it!=nullptr) {
        it->runTests();
    }
}

void RunTime::injectionPoint_end(std::string pname, std::string id) {
    auto it= getExistingInjectionPoint(pname,id,InjectionPointType::End,"End");
    if (it != nullptr) {
        it->runTests();
    }
}

RunTime& RunTime::instance() {
  static RunTime store;
  return store;
}

RunTime::RunTime() {}

void RunTime::registerLogLevel(std::string name, std::string color) {
    logger.registerLogLevel(name,color);
}

void RunTimeOptions::callback(json &j) {
    RunTime::instance().runTimeOptions.fromJson(j);
}


void RunTimeOptions::fromJson(json& j) {
    if (j.contains("logUnhandled")) {
        logUnhandled = j["logUnhandled"].get<bool>();
    }
}


void RunTime::loadRunInfo(RunInfo &info, registrationCallBack *callback) {

  // Set up the logger. This occurs as early as possible to allow log messages
  // to be caught int the registration objects.
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
  VnV_Register_Options(getBaseOptionsSchema(), RunTimeOptions::callback);

  // Load the test libraries ( plugins -- This could include a custom engine )
  VnV_Debug("Loading Test Libraries");
  for (auto it : info.testLibraries) {
      if (!it.second.empty()) {
        TestStore::getTestStore().addTestLibrary(it.second);
      }
      VnV_Debug("\t Loaded Test Library %s" , it.second.c_str());
  }
  // Now we need to call the library registration functions.
  makeLibraryRegistrationCallbacks(info.testLibraries);

  // Call the executables registration callback.
  if (callback != nullptr) {
      (*callback)();
  }

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.toolConfig);


  if (!OutputEngineStore::getOutputEngineStore().isInitialized()) {
      VnV_Debug("Configuring The Output Engine");
      OutputEngineStore::getOutputEngineStore().setEngineManager(
            info.engineInfo.engineType, info.engineInfo.engineConfig);
      VnV_Debug("Output Engine Configuration Successful");
  }

  std::map<std::string, std::pair<bool, std::vector<TestConfig>>> configs;
  VnV_Debug("Validating Json Test Configuration Input and converting to TestConfig objects");
  for ( auto it : info.injectionPoints) {
      configs.insert(std::make_pair(it.first, std::make_pair(it.second.runInternal, TestStore::getTestStore().validateTests(it.second.tests)))) ;
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

namespace {
bool initMPI(int* argc, char*** argv) {
  int flag = 0;

#ifdef WITH_MPI
  MPI_Initialized(&flag);
  if (!flag) {
    MPI_Init(argc, argv);
    return true;
  }
  return false;
#endif

}

}
// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(int* argc, char*** argv, json& config, registrationCallBack *callback) {
  finalize_mpi = initMPI(argc,argv);

  JsonParser parser;
  RunInfo info = parser.parse(config);
  runTests = configure(info,callback);

  INJECTION_LOOP_BEGIN_C(VnV_Comm_World, initialization, [&](VnV_Comm comm, VnVParameterSet &p, OutputEngineManager *engine){
           for (auto it : p ) {
               std::string t = it.second.getType() + "(rtti:" + it.second.getRtti() + ")";
               engine->Put(comm, it.first, t);
           }
  },argc, argv, config);
}

bool RunTime::InitFromFile(int* argc, char*** argv, std::string configFile, registrationCallBack *callback) {

  std::ifstream fstream(configFile);

  json mainJson;
  if (!fstream.good()) {
    throw VnVExceptionBase("Invalid Input File Stream");
  }

  try {
     mainJson = json::parse(fstream);
  } catch (json::exception e) {
     throw VnVExceptionBase(e.what());
  }

  return InitFromJson(argc,argv,mainJson,callback);

}

bool RunTime::configure(RunInfo info, registrationCallBack *callback) {

  runTests = info.runTests;
  if (runTests) {
      loadRunInfo(info,callback);

      if (info.unitTestInfo.runUnitTests) {
          runUnitTests(VnV_Comm_World);
      }

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
    INJECTION_LOOP_END(initialization);
    OutputEngineStore::getOutputEngineStore().getEngineManager()->finalize();
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

void RunTime::documentationPoint(VnV_Comm comm, std::string pname, std::string id, NTV &map){
    OutputEngineStore::getOutputEngineStore().getEngineManager()->document(comm, pname, id, map);
}

void RunTime::log(VnV_Comm comm, std::string pname, std::string level, std::string message, va_list args) {
    logger.log_c(comm,pname, level, message, args);
}



int RunTime::beginStage(VnV_Comm comm, std::string pname, std::string message, va_list args) {
 return   logger.beginStage(comm,pname, message, args);
}

void RunTime::endStage(VnV_Comm comm, int ref) {
    logger.endStage(comm,ref);
}

void RunTime::runUnitTests(VnV_Comm comm) {
  UnitTestStore::getUnitTestStore().runAll(comm, false);
}

void RunTime::readFile(std::string filename){
    OutputEngineStore::getOutputEngineStore().getEngineManager()->readFromFile(filename);
}
void RunTime::printRunTimeInformation() {
        int a = VnV_BeginStage("Runtime Configuration");
        logger.print();
        OutputEngineStore::getOutputEngineStore().print();
        TestStore::getTestStore().print();
        InjectionPointStore::getInjectionPointStore().print();
        VnV_EndStage(a);
}






