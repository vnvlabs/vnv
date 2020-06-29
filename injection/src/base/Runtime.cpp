
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include <unistd.h>

//#include <unistd.h>
#include "base/CommunicationStore.h"
#include "base/DistUtils.h"
#include "base/InjectionPointStore.h"
#include "base/JsonSchema.h"
#include "base/OptionsParserStore.h"
#include "base/OutputEngineStore.h"
#include "base/Runtime.h"
#include "base/TestStore.h"
#include "base/UnitTestStore.h"
#include "c-interfaces/Logging.h"

#include <iostream>

using namespace VnV;

INJECTION_OPTIONS(getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
}

void RunTime::loadPlugin(std::string libraryPath, std::string packageName) {
  try {
    auto it = plugins.find(libraryPath);
    if (it == plugins.end()) {
      void* dllib = DistUtils::loadLibrary(libraryPath);
      if (dllib != nullptr) {
        registrationCallBack reg = DistUtils::searchLibrary(dllib, packageName);
        if (reg != nullptr) {
          runTimePackageRegistration(packageName, reg);
        } else {
          std::cout << "GERE" << std::endl;
        }
      } else {
        VnV_Warn("Library not found");
      }
    }
  } catch (...) {
    VnV_Warn("Library not found: %s", libraryPath.c_str());
  }
}

void RunTime::makeLibraryRegistrationCallbacks(
    std::map<std::string, std::string> packageNames) {
  for (auto it : packageNames) {
    loadPlugin(it.second, it.first);
  }
}

bool RunTime::useAsciiColors() { return terminalSupportsAsciiColors; }

void RunTime::logUnhandled(std::string name, std::string id, NTV& args) {
  int a = VnV_BeginStage("Unhandled Injection Point");
  VnV_Info("Name: %s", name.c_str());
  VnV_Info("ID: %s", id.c_str());
  int aa = VnV_BeginStage("Parameters");
  for (auto& it : args) {
    VnV_Info("%s : (%s)", it.first.c_str(), it.second.first.c_str());
  }
  VnV_EndStage(aa);
  VnV_EndStage(a);
}

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(
    std::string pname, std::string id, InjectionPointType type, NTV& args) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getNewInjectionPoint(
            pname, id, type, args);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, args);
    }
  }
  return nullptr;
}

std::shared_ptr<InjectionPoint> RunTime::getExistingInjectionPoint(
    std::string pname, std::string id, InjectionPointType type,
    std::string stageId) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getExistingInjectionPoint(
            pname, id, type);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, stageId);
      return ipd;
    }
  }
  return nullptr;
}

void RunTime::writeSpecification(std::string filename) {
     std::ofstream ofs(filename);
     if (ofs.good()) {
        ofs << getFullJson().dump(4);
     } else {
        std::cout << getFullJson().dump(4) << std::endl;
     }
}

nlohmann::json RunTime::getFullJson(){
  json main = json::object();
  for (auto &package : jsonCallbacks) {
     json j = json::parse(package.second());
     for (auto type : j.items()) {
        json &mj = JsonUtilities::getOrCreate(main,type.key(),JsonUtilities::CreateType::Object);
        if (type.key() == "Options" ) {
            json jj = type.value();
            jj["config"] = OptionsParserStore::instance().getSchema(package.first);
            mj[package.first] = jj;
          } else if (type.key() == "DataType") {
            for (auto &entry : type.value().items()) {
               mj[entry.key()] = entry.value();
            }
          }
        else {
             for (auto &entry : type.value().items() ) {
                mj[package.first + ":" + entry.key()] = entry.value();
             }
        }
     }
  }
  return main;
}

RunTimeOptions* RunTime::getRunTimeOptions() { return &runTimeOptions; }

// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             const CppInjection::DataCallback& callback,
                             NTV& args) {
  std::cout << "INJECTION " << pname << " : " << id << std::endl;
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(comm);
    it->runTests();
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id,
                                   const CppInjection::DataCallback& callback,
                                   NTV& args) {
  std::cout << "INJECTION " << pname << " : " << id << std::endl;
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(comm);
    it->runTests();
  }
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             injectionDataCallback* callback, NTV& args) {
  std::cout << "INJECTION " << pname << " : " << id << std::endl;
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(comm);
    it->runTests();
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id,
                                   injectionDataCallback* callback, NTV& args) {
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(comm);
    it->runTests();
  }
}

void RunTime::injectionPoint_iter(std::string pname, std::string id,
                                  std::string stageId) {
  auto it =
      getExistingInjectionPoint(pname, id, InjectionPointType::Iter, stageId);
  if (it != nullptr) {
    it->runTests();
  }
}

void RunTime::injectionPoint_end(std::string pname, std::string id) {
  auto it =
      getExistingInjectionPoint(pname, id, InjectionPointType::End, "End");
  if (it != nullptr) {
    it->runTests();
  }
}

void RunTime::declareCommunicator(std::string packageName, std::string commpack,
                                  std::string communicator) {
  CommunicationStore::instance().declareComm(packageName, commpack,
                                             communicator);
}

void RunTime::declarePackageJson(std::string pname,
                                 vnvFullJsonStrCallback callback) {
  jsonCallbacks.insert(std::make_pair(pname, callback));
}

RunTime& RunTime::instance() {
  static RunTime store;
  return store;
}

RunTime::RunTime() {}

void RunTime::registerLogLevel(std::string packageName, std::string name,
                               std::string color) {
  logger.registerLogLevel(packageName, name, color);
}

void RunTimeOptions::callback(json& j) {
  RunTime::instance().runTimeOptions.fromJson(j);
}

void RunTimeOptions::fromJson(json& j) {
  std::cout << j.dump(3);
  if (j.contains("logUnhandled")) {
    logUnhandled = j["logUnhandled"].get<bool>();
  }
  if (j.contains("dumpConfig")) {
     dumpConfigFilename = j["dumpConfig"].get<std::string>();
     dumpConfig = true;
  }
  if (j.contains("command-line")) {
     json& cmd = j["command-line"];
     if (cmd.contains("dumpConfig")) {
        dumpConfig = true;
        dumpConfigFilename = cmd["dumpConfig"].get<std::string>();
     }
     if (cmd.contains("logUnhandled")) {
        logUnhandled = true;
     }
  }
}

void RunTime::loadRunInfo(RunInfo& info, registrationCallBack* callback) {
  // Set up the logger. This occurs as early as possible to allow log messages
  // to be caught int the registration objects.
  if (info.logInfo.on) {
    logger.setLog(info.logInfo.filename);
    for (auto it : info.logInfo.logs) {
      logger.setLogLevel(it.first, it.second);
    }
    for (auto it : info.logInfo.blackList) {
      logger.addToBlackList(it);
    }
  }

  // Register this library -- the VnV Toolkit.
  runTimePackageRegistration(PACKAGENAME_S, INJECTION_REGISTRATION_PTR);

  // Register the Executable.
  if (callback != nullptr) {
    runTimePackageRegistration(mainPackageName, *callback);
  }

  // Register the plugins specified in the input file.
  makeLibraryRegistrationCallbacks(info.additionalPlugins);

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.pluginConfig,info.cmdline);

  if (getRunTimeOptions()->dumpConfig) {
     writeSpecification(getRunTimeOptions()->dumpConfigFilename);
  }

  if (!OutputEngineStore::getOutputEngineStore().isInitialized()) {
    VnV_Debug("Configuring The Output Engine");
    OutputEngineStore::getOutputEngineStore().setEngineManager(
        info.engineInfo.engineType, info.engineInfo.engineConfig);
    VnV_Debug("Output Engine Configuration Successful");
  }

  VnV_Debug(
      "Validating Json Test Configuration Input and converting to TestConfig "
      "objects");
  for (auto it : info.injectionPoints) {
    auto x = std::make_pair(
        it.second.runInternal,
        TestStore::getTestStore().validateTests(it.second.tests));
    InjectionPointStore::getInjectionPointStore().addInjectionPoint(
        it.second.package, it.second.name, x);
  }
}

void RunTime::loadInjectionPoints(json _json) {
  JsonParser parser;
  char** argv;
  int argc = 0;
  try {
     RunInfo info = parser.parse(_json, &argc, argv);
     loadRunInfo(info, nullptr);
  } catch (VnVExceptionBase e) {
     std::cout << "Loading of injection points failed" << std::endl;
     std::cout << e.what() << std::endl;
     return ;
  }
}



// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv,
                           json& config, registrationCallBack* callback) {
  mainPackageName = packageName;

  JsonParser parser;
  RunInfo info;
  try {
    info = parser.parse(config,argc,*argv);
  } catch (VnVExceptionBase e) {
     std::cerr << "VnV Initialization Failed during input file validation. \n";
     std::cerr << e.what() << std::endl;;
     std::abort();
  }
  runTests = configure(packageName, info, callback);

  /**
   * Injection point documentation.
   **/

  VnV_Comm comm = CommunicationStore::instance().worldData(packageName);

  INJECTION_LOOP_BEGIN_C(
      comm, initialization,
      [&](VnV_Comm comm, VnVParameterSet& p, OutputEngineManager* engine) {
        for (auto it : p) {
          std::string t =
              it.second.getType() + "(rtti:" + it.second.getRtti() + ")";
          engine->Put(comm, it.first, t);
        }
      },
      argc, argv, config);

  return runTests;
}

bool RunTime::InitFromFile(const char* packageName, int* argc, char*** argv,
                           std::string configFile,
                           registrationCallBack* callback) {
  std::ifstream fstream(configFile);


  json mainJson;
  if (!fstream.good()) {
    throw Exceptions::fileReadError(configFile);
  }

  try {
    mainJson = json::parse(fstream);
  } catch (json::parse_error e) {
    throw Exceptions::parseError(fstream,e.byte,e.what());
  }

  return InitFromJson(packageName, argc, argv, mainJson, callback);
}

bool RunTime::configure(std::string packageName, RunInfo info,
                        registrationCallBack* callback) {
  runTests = info.runTests;
  if (runTests) {
    loadRunInfo(info, callback);

    if (info.unitTestInfo.runUnitTests) {
      runUnitTests(VnV_Comm_World(packageName.c_str()));
    }

  } else if (info.error) {
    runTests = false;
    processToolConfig(info.pluginConfig,info.cmdline);
  }

  return runTests;
}

void RunTime::processToolConfig(json config, json& cmdline) {
  OptionsParserStore::instance().parse(config, cmdline);
}

void RunTime::runTimePackageRegistration(std::string packageName,
                                         vnv_registration_function reg) {
  auto it = plugins.find(packageName);
  if (it == plugins.end()) {
    VnV_Debug("Registering a new Package %s", packageName.c_str());
    plugins.insert(packageName);
    reg();  // could be recursive.
  }
}

bool RunTime::Finalize() {
  if (runTests) {
    INJECTION_LOOP_END(initialization);
    OutputEngineStore::getOutputEngineStore().getEngineManager()->finalize();
  }
  return true;
}

bool RunTime::isRunTests() { return runTests; }

void RunTime::log(VnV_Comm comm, std::string pname, std::string level,
                  std::string message, va_list args) {
  logger.log_c(comm, pname, level, message, args);
}

int RunTime::beginStage(VnV_Comm comm, std::string pname, std::string message,
                        va_list args) {
  return logger.beginStage(comm, pname, message, args);
}

void RunTime::endStage(VnV_Comm comm, int ref) { logger.endStage(comm, ref); }

void RunTime::runUnitTests(VnV_Comm comm) {
  UnitTestStore::getUnitTestStore().runAll(comm, false);
}

void RunTime::readFile(std::string filename) {
  OutputEngineStore::getOutputEngineStore().getEngineManager()->readFromFile(
      filename);
}

void RunTime::printRunTimeInformation() {
  int a = VnV_BeginStage("Runtime Configuration");
  logger.print();
  OutputEngineStore::getOutputEngineStore().print();
  TestStore::getTestStore().print();
  InjectionPointStore::getInjectionPointStore().print();
  VnV_EndStage(a);
}


