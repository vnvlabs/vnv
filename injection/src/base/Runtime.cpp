
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#ifdef WITH_MPI
#  include <mpi.h>
#endif

#include <unistd.h>

//#include <unistd.h>
#include "base/Runtime.h"
#include "base/TestStore.h"
#include "base/DistUtils.h"
#include "base/InjectionPointStore.h"
#include "base/JsonSchema.h"
#include "base/OutputEngineStore.h"
#include "base/OptionsParserStore.h"
#include "base/UnitTestStore.h"
#include "c-interfaces/Logging.h"

using namespace VnV;


INJECTION_OPTIONS(getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
}

void RunTime::loadPlugin(std::string libraryPath, std::string packageName) {
  try {
    auto it = plugins.find(libraryPath);
    if (it == plugins.end() ) {
        void* dllib = DistUtils::loadLibrary(libraryPath);
        if (dllib != nullptr) {
           registrationCallBack reg =  DistUtils::searchLibrary(dllib, packageName);
           if (reg != nullptr) {
              runTimePackageRegistration(packageName,reg);
           }  else {
              std::cout << "GERE" << std::endl;
           }
          } else {
            VnV_Warn("Library not found");
          }
    }
  } catch (...) {
    VnV_Warn("Library not found: %s",libraryPath.c_str());
  }
}

void RunTime::makeLibraryRegistrationCallbacks(std::map<std::string, std::string> packageNames) {
  for ( auto it : packageNames) {
     loadPlugin(it.second,it.first);
  }
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
        InjectionPointStore::getInjectionPointStore().getNewInjectionPoint(pname, id,type,args);
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
        InjectionPointStore::getInjectionPointStore().getExistingInjectionPoint(pname,id,type);
    if (ipd != nullptr) {
        ipd->setInjectionPointType(type,stageId);
        return ipd;
    }
  }
  return nullptr;
}

RunTimeOptions *RunTime::getRunTimeOptions() {
  return &runTimeOptions;
}


// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, const CppInjection::DataCallback &callback, NTV &args) {
    std::cout << "INJECTION " << pname << " : " << id << std::endl;
    auto it = getNewInjectionPoint(pname,id,InjectionPointType::Single,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, const CppInjection::DataCallback &callback, NTV &args) {
    std::cout << "INJECTION " << pname << " : " << id << std::endl;
    auto it  =getNewInjectionPoint(pname,id,InjectionPointType::Begin,args);
    if ( it != nullptr) {
        it->setCallBack(callback);
        it->setComm(comm);
        it->runTests();
    }
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, injectionDataCallback *callback, NTV &args) {
    std::cout << "INJECTION " << pname << " : " << id << std::endl;
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

void RunTime::declarePackageJson(std::string pname, vnvFullJsonStrCallback callback) {
   jsonCallbacks.insert(std::make_pair(pname,callback));
}

RunTime& RunTime::instance() {
  static RunTime store;
  return store;
}

RunTime::RunTime() {}

void RunTime::registerLogLevel(std::string packageName, std::string name, std::string color) {
    logger.registerLogLevel(packageName, name,color);
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

  //Register this library -- the VnV Toolkit.
  runTimePackageRegistration(PACKAGENAME_S,INJECTION_REGISTRATION_PTR);

  //Register the Executable.
  if (callback != nullptr) {
      runTimePackageRegistration(mainPackageName, *callback);
  }

  //Register the plugins specified in the input file. In this case,
  //we use dlopen to open the library dynamically, then search the library
  //for a registration function called VNV_REGISTRATION_NAME_<PACKAGENAME>
  makeLibraryRegistrationCallbacks(info.testLibraries);


  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.toolConfig);


  if (!OutputEngineStore::getOutputEngineStore().isInitialized()) {
      VnV_Debug("Configuring The Output Engine");
      OutputEngineStore::getOutputEngineStore().setEngineManager(
            info.engineInfo.engineType, info.engineInfo.engineConfig);
      VnV_Debug("Output Engine Configuration Successful");
  }

  VnV_Debug("Validating Json Test Configuration Input and converting to TestConfig objects");
  for ( auto it : info.injectionPoints) {
      auto x = std::make_pair(it.second.runInternal,TestStore::getTestStore().validateTests(it.second.tests));
      InjectionPointStore::getInjectionPointStore().addInjectionPoint(
                  it.second.package, it.second.name,x);
  }

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
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv, json& config, registrationCallBack *callback) {
  mainPackageName = packageName;
  finalize_mpi = initMPI(argc,argv);

  JsonParser parser;
  RunInfo info = parser.parse(config);
  runTests = configure(info,callback);

  /**
   * Injection point documentation.
   **/
  INJECTION_LOOP_BEGIN_C(VnV_Comm_World, initialization, [&](VnV_Comm comm, VnVParameterSet &p, OutputEngineManager *engine){
           for (auto it : p ) {
               std::string t = it.second.getType() + "(rtti:" + it.second.getRtti() + ")";
               engine->Put(comm, it.first, t);
           }
  },argc, argv, config);

  return runTests;
}

bool RunTime::InitFromFile(const char* packageName, int* argc, char*** argv, std::string configFile, registrationCallBack *callback) {

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

  return InitFromJson(packageName, argc,argv,mainJson,callback);

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

void RunTime::runTimePackageRegistration(std::string packageName, vnv_registration_function reg) {

  auto it = plugins.find(packageName);
  if ( it == plugins.end() ) {
      VnV_Debug("Registering a new Package %s" , packageName.c_str());
      plugins.insert(packageName);
      reg() ; // could be recursive.
    }
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




