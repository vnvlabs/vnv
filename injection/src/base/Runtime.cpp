
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include <unistd.h>

//#include <unistd.h>
#include <iostream>

#include "base/CommunicationStore.h"
#include "base/DistUtils.h"
#include "base/InjectionPointStore.h"
#include "base/JsonSchema.h"
#include "base/OptionsParserStore.h"
#include "base/OutputEngineStore.h"
#include "base/Runtime.h"
#include "base/TestStore.h"
#include "base/UnitTestStore.h"
#include "base/Utilities.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

INJECTION_OPTIONS(VNVPACKAGENAME, getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
}

namespace {
ICommunicator_ptr getComm(VnV_Comm comm) {
  return CommunicationStore::instance().customComm(comm.name, comm.data);
}
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
        }
      } else {
        VnV_Warn(VNVPACKAGENAME, "Library not found");
      }
    }
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME, "Library not found: %s", libraryPath.c_str());
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
  int a = VnV_BeginStage(VNVPACKAGENAME, "Unhandled Injection Point");
  VnV_Info(VNVPACKAGENAME, "Name: %s", name.c_str());
  VnV_Info(VNVPACKAGENAME, "ID: %s", id.c_str());
  int aa = VnV_BeginStage(VNVPACKAGENAME, "Parameters");
  for (auto& it : args) {
    VnV_Info(VNVPACKAGENAME, "%s : (%s)", it.first.c_str(),
             it.second.first.c_str());
  }
  VnV_EndStage(VNVPACKAGENAME, aa);
  VnV_EndStage(VNVPACKAGENAME, a);
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
  }
}

nlohmann::json RunTime::getFullJson() {
  json main = json::object();

  for (auto& package : jsonCallbacks) {
    json j = json::parse(package.second());
    for (auto type : j.items()) {
      // Add all the options and stuff
      json& mj = JsonUtilities::getOrCreate(main, type.key(),
                                            JsonUtilities::CreateType::Object);
      if (type.key() == "Options") {
        json jf = OptionsParserStore::instance().getSchema(package.first);
        jf["docs"] = type.value();
        mj[package.first] = jf;
      } else if (type.key() == "DataType") {
        for (auto& entry : type.value().items()) {
          mj[entry.key()] = entry.value();
        }
      } else if (type.key() == "Introduction" || type.key() == "Conclusion") {
        if (package.first == mainPackageName) {
          mj["docs"] = type.value();
        }
      } else {
        for (auto& entry : type.value().items()) {
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
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->runTests();
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id,
                                   const CppInjection::DataCallback& callback,
                                   NTV& args) {
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->runTests();
  }
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             injectionDataCallback* callback, NTV& args) {
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->runTests();
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id,
                                   injectionDataCallback* callback, NTV& args) {
  auto it = getNewInjectionPoint(pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
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
  runTimePackageRegistration(VNV_STR(VNVPACKAGENAME),
                             INJECTION_REGISTRATION_PTR(VNVPACKAGENAME));

  // Register the Executable.
  if (callback != nullptr) {
    runTimePackageRegistration(mainPackageName, *callback);
  }

  // Register the plugins specified in the input file.
  makeLibraryRegistrationCallbacks(info.additionalPlugins);

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.pluginConfig, info.cmdline);

  if (getRunTimeOptions()->dumpConfig) {
    writeSpecification(getRunTimeOptions()->dumpConfigFilename);
  }

  if (!OutputEngineStore::getOutputEngineStore().isInitialized()) {
    VnV_Debug(VNVPACKAGENAME, "Configuring The Output Engine");
    OutputEngineStore::getOutputEngineStore().setEngineManager(
        info.engineInfo.engineType, info.engineInfo.engineConfig);
    VnV_Debug(VNVPACKAGENAME, "Output Engine Configuration Successful");
  }

  VnV_Debug(
      VNVPACKAGENAME,
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
  char** argv = nullptr;
  int argc = 0;
  try {
    RunInfo info = parser.parse(_json, &argc, argv);
    loadRunInfo(info, nullptr);
  } catch (VnVExceptionBase e) {
    std::cerr << "Loading of injection points failed" << std::endl;
    std::cerr << e.what() << std::endl;
    std::abort();
  }
}

// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv,
                           json& config, registrationCallBack* callback) {
  mainPackageName = packageName;

  JsonParser parser;
  RunInfo info;
  try {
    info = parser.parse(config, argc, *argv);
  } catch (VnVExceptionBase e) {
    std::cerr << "VnV Initialization Failed during input file validation. \n";
    std::cerr << e.what() << std::endl;
    ;
    std::abort();
  }
  runTests = configure(packageName, info, callback);

  /**
   * Injection point documentation.
   **/


  VnV_Comm comm = CommunicationStore::instance().worldData(packageName);


  /**
   * VnV Configuration and provenance Tracking information.
   * ======================================================
   *
   * CmdLine:  :vnv:`Data.Data."command-line"`.
   *
   * Time of execution :vnv:`Data.Data.time`.
   * The VnV Configuration file was
   *
   * .. vnv-jchart::
   *   :main: Data.Data.config.Value
   *
   *   $$main$$
   *
   *
   * This injection point is called at the end of the VnVInit function
   * to allow users to collect provenance information about the executable.
   *
   */
  INJECTION_POINT_C(
      VNVPACKAGENAME, comm, configuration,
      [&](VnV_Comm comm, VnVParameterSet& p, OutputEngineManager* engine,
          InjectionPointType type, std::string stageId) {
        // caught everything, so internal ones can ignore parameters and put
        // anything.
        // Here, we do a minimal provenance history. For a full history, the
        // user should attach the provenace test to this node.
        auto config = p["config"].getByRtti<json>();
        auto argc = p["argc"].getByRtti<int*>();
        auto argv = p["argv"].getByRtti<char***>();
        std::cout <<"aaaaSDFSDFSDFSDFSDFSDFSDF" << std::endl;

        std::string currTime = VnV::ProvenanceUtils::timeToString();
        std::string commandline =
            VnV::ProvenanceUtils::cmdLineToString(*argc, *argv);
        engine->Put( "config", config);
        engine->Put( "command-line", commandline);
        engine->Put( "time", currTime);

        std::cout <<"SDFSDFSDFSDFSDFSDFSDF" << std::endl;

      },
      argc, argv, config);

  /**
   *VnV Application Profiling Loop.
   *===============================
   *
   *This injection point is called at the end of the VnVInit function. This is a
   *looped injection point with no interesting parameters passed in. This
   *injection point exists soley as a mechanism for profiling the given
   *application between the VnVInit and VnVFinalize functions.
   *
   */
  INJECTION_LOOP_BEGIN(VNVPACKAGENAME, comm, initialization, runTests);

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
    throw Exceptions::parseError(fstream, e.byte, e.what());
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
    processToolConfig(info.pluginConfig, info.cmdline);
  }
  std::cout << "DSFSDFSDFDFSDFDSF" << std::endl;
  return runTests;
}

void RunTime::processToolConfig(json config, json& cmdline) {
  OptionsParserStore::instance().parse(config, cmdline);
}

void RunTime::runTimePackageRegistration(std::string packageName,
                                         vnv_registration_function reg) {
  auto it = plugins.find(packageName);
  if (it == plugins.end()) {
    VnV_Debug(VNVPACKAGENAME, "Registering a new Package %s",
              packageName.c_str());
    plugins.insert(packageName);
    reg();  // could be recursive.
  }
}

bool RunTime::Finalize() {
  if (runTests) {
    auto comm = CommunicationStore::instance().worldComm(mainPackageName);
    INJECTION_LOOP_END(VNVPACKAGENAME, initialization);
    OutputEngineStore::getOutputEngineStore().getEngineManager()->finalize(comm);
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

void RunTime::readFile(std::string filename, long& idCounter) {
  OutputEngineStore::getOutputEngineStore().getEngineManager()->readFromFile(
      filename, idCounter);
}

void RunTime::printRunTimeInformation() {
  int a = VnV_BeginStage(VNVPACKAGENAME, "Runtime Configuration");
  logger.print();
  OutputEngineStore::getOutputEngineStore().print();
  TestStore::getTestStore().print();
  InjectionPointStore::getInjectionPointStore().print();
  VnV_EndStage(VNVPACKAGENAME, a);
}
