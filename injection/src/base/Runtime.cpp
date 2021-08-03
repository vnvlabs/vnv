
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include <unistd.h>

//#include <unistd.h>
#include <Python.h>

#include <iostream>

#include "base/DistUtils.h"
#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/parser/JsonSchema.h"
#include "base/stores/ActionStore.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/InjectionPointStore.h"
#include "base/stores/IteratorStore.h"
#include "base/stores/IteratorsStore.h"
#include "base/stores/OptionsParserStore.h"
#include "base/stores/OutputEngineStore.h"
#include "base/stores/PlugStore.h"
#include "base/stores/PlugsStore.h"
#include "base/stores/TestStore.h"
#include "base/stores/UnitTestStore.h"
#include "c-interfaces/Logging.h"
#include "interfaces/IAction.h"
#include "interfaces/points/Injection.h"

using namespace VnV;

INJECTION_OPTIONS(VNVPACKAGENAME, getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
}

namespace {
  
ICommunicator_ptr getComm(VnV_Comm comm) {
  return CommunicationStore::instance().getCommunicator(comm);
}
}  // namespace

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

int RunTime::registerCleanUpAction(
    std::function<void(ICommunicator_ptr)> action) {
  int i = cleanupActionCounter++;
  cleanupActions[i] = action;
  return i;
}

void RunTime::makeLibraryRegistrationCallbacks(
    std::map<std::string, std::string> packageNames) {
  for (auto it : packageNames) {
    loadPlugin(it.second, it.first);
  }
}

bool RunTime::useAsciiColors() { return terminalSupportsAsciiColors; }

void RunTime::logUnhandled(std::string name, std::string id, NTV& args) {
  VnV_Info(VNVPACKAGENAME, "Name: %s", name.c_str());
  VnV_Info(VNVPACKAGENAME, "ID: %s", id.c_str());
  for (auto& it : args) {
    VnV_Info(VNVPACKAGENAME, "%s : (%s)", it.first.c_str(),
             it.second.first.c_str());
  }
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
  main.merge_patch(template_patch);
  return main;
}

RunTimeOptions* RunTime::getRunTimeOptions() { return &runTimeOptions; }

/*********************** ITERATIONS ************************************/

namespace {
class VnV_Iterator_Info {
 public:
  std::string name;
  std::string pname;
  int once;
  int count = 0;
  std::shared_ptr<IterationPoint> iter;
  VnV_Iterator_Info(std::string n, std::string p, int o,
                    std::shared_ptr<IterationPoint>& it)
      : name(n), pname(p), once(o), iter(it) {}
};

}  // namespace

std::shared_ptr<IterationPoint> RunTime::getNewInjectionIteration(VnV_Comm comm, 
    std::string pname, std::string id, InjectionPointType type, int once,
    NTV& in_args, NTV& out_args) {
  if (runTests) {
    // Load any hot patches
    loadHotPatch(comm);

    std::shared_ptr<IterationPoint> ipd =
        IteratorStore::instance().getNewIterator(pname, id, once, in_args,
                                                 out_args);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, in_args);
    }
  }
  return nullptr;
}

VnV_Iterator RunTime::injectionIteration(VnV_Comm comm, std::string pname,
                                         std::string id,
                                         const DataCallback& callback,
                                         NTV& inputs, NTV& outputs, int once) {
  auto it = getNewInjectionIteration(comm, pname, id, InjectionPointType::Begin, once,
                                     inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
    it->iterate();
  }
  VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, it);
  return {(void*)info};
}

VnV_Iterator RunTime::injectionIteration(VnV_Comm comm, std::string pname,
                                         std::string id,
                                         injectionDataCallback* callback,
                                         NTV& inputs, NTV& outputs, int once) {
  auto it = getNewInjectionIteration(comm, pname, id, InjectionPointType::Begin, once,
                                     inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
    it->iterate();
  }
  VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, it);
  return {(void*)info};
}

int RunTime::injectionIterationRun(VnV_Iterator* iterator) {
  VnV_Iterator_Info* info = (VnV_Iterator_Info*)iterator->data;
  if (info->iter != nullptr) {
    return info->iter->iterate();
  } else {
    if (info->count < info->once) {
      info->count++;
      return 1;
    } else {
      return 0;
    }
  }
}

/**************************** PLUGS *******************************/

namespace {
class VnV_Plug_Info {
 public:
  std::string name;
  std::string pname;
  std::shared_ptr<PlugPoint> iter;
  VnV_Plug_Info(std::string n, std::string p, std::shared_ptr<PlugPoint>& it)
      : name(n), pname(p), iter(it) {}
};
}  // namespace

std::shared_ptr<PlugPoint> RunTime::getNewInjectionPlug(VnV_Comm comm,
                                                        std::string pname,
                                                        std::string id,
                                                        NTV& in_args,
                                                        NTV& out_args) {
  if (runTests) {
    // load hotpatches
    loadHotPatch(comm);

    std::shared_ptr<PlugPoint> ipd =
        PlugStore::getPlugStore().getNewPlug(pname, id, in_args, out_args);
    if (ipd != nullptr) {
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, in_args);
    }
  }
  return nullptr;
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname,
                                    std::string id,
                                    const DataCallback& callback, NTV& inputs,
                                    NTV& outputs) {
  auto it = getNewInjectionPlug(comm, pname, id, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
  }
  VnV_Plug_Info* info = new VnV_Plug_Info(id, pname, it);
  return {(void*)info};
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname,
                                    std::string id,
                                    injectionDataCallback* callback,
                                    NTV& inputs, NTV& outputs) {
  auto it = getNewInjectionPlug(comm, pname, id, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
  }
  VnV_Plug_Info* info = new VnV_Plug_Info(id, pname, it);
  return {(void*)info};
}

int RunTime::injectionPlugRun(VnV_Iterator* iterator) {
  VnV_Plug_Info* info = (VnV_Plug_Info*)iterator->data;

  if (info->iter != nullptr) {
    info->iter->plug();
    return 1;
  } else {
    return 0;
  }
}

/************************** INJECTION POINTS
 * ****************************************/

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(VnV_Comm comm, 
    std::string pname, std::string id, InjectionPointType type, NTV& in_args) {
  if (runTests) {
    // look for hotpatches;
    loadHotPatch(comm);

    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::getInjectionPointStore().getNewInjectionPoint(
            pname, id, type, in_args);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, in_args);
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

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id, const DataCallback& callback,
                                   NTV& args) {
  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run();
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id,
                                   injectionDataCallback* callback, NTV& args) {
  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run();
  }
}

// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             const DataCallback& callback, NTV& args) {
  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run();
  }
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             injectionDataCallback* callback, NTV& args) {
  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run();
  }
}

void RunTime::injectionPoint_iter(std::string pname, std::string id,
                                  std::string stageId) {
  auto it =
      getExistingInjectionPoint(pname, id, InjectionPointType::Iter, stageId);
  if (it != nullptr) {
    it->run();
  }
}

void RunTime::injectionPoint_end(std::string pname, std::string id) {
  auto it =
      getExistingInjectionPoint(pname, id, InjectionPointType::End, "End");
  if (it != nullptr) {
    it->run();
  }
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
    exitAfterDumpConfig = j.contains("exitAfterDumpConfig") &&
                          j["exitAfterDumpConfig"].get<bool>();
  }
}

void RunTime::getFullSchema(std::string filename) {}

void RunTime::loadRunInfo(RunInfo& info, registrationCallBack* callback) {
  initializedCount++;

  if (initializedCount == 1) {
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

    // Pull out the template patch. This is the user provides specification file
    // that should be merged into the final specificiation.
    template_patch = info.template_overrides;

    //Register VnV. 
    runTimePackageRegistration(VNV_STR(VNVPACKAGENAME),
                               INJECTION_REGISTRATION_PTR(VNVPACKAGENAME));

    // Register the Executable.
    if (callback != nullptr) {
      runTimePackageRegistration(mainPackageName, *callback);
    }

    // Now we register VnVs communicator. The VnV communicator is set to be the same as the communicator
    // for the executable.
    //CommunicationStore::instance().copySettings(mainPackageName,VNVPACKAGENAME_S);

  }

  // Register the plugins specified in the input file.
  makeLibraryRegistrationCallbacks(info.additionalPlugins);

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.pluginConfig, info.cmdline);

  if (getRunTimeOptions()->dumpConfig) {
    writeSpecification(getRunTimeOptions()->dumpConfigFilename);
    if (getRunTimeOptions()->exitAfterDumpConfig) {
      return;
    }
  }

  ICommunicator_ptr world = CommunicationStore::instance().worldComm();

  if (initializedCount == 1) {

     if (!OutputEngineStore::getOutputEngineStore().isInitialized()) {
      VnV_Debug(VNVPACKAGENAME, "Configuring The Output Engine");
      OutputEngineStore::getOutputEngineStore().setEngineManager(
          world, info.engineInfo.engineType, info.engineInfo.engineConfig);
      VnV_Debug(VNVPACKAGENAME, "Output Engine Configuration Successful");
    }
  }

  hotpatch = info.hotpatch;

  VnV_Debug(
      VNVPACKAGENAME,
      "Validating Json Test Configuration Input and converting to TestConfig "
      "objects");

  for (auto it : info.injectionPoints) {

    auto x = TestStore::instance().validateTests(it.second.tests);

    if (it.second.type == InjectionType::ITER) {
      auto iterations = IteratorsStore::instance().validateTests(it.second.iterators);
      IteratorStore::instance().addIterator(it.second.package, it.second.name,
                                            it.second.runInternal, x,
                                            iterations);

    } else if (it.second.type == InjectionType::PLUG) {
      auto plug = std::make_shared<PlugConfig>(
          std::move(PlugsStore::instance().validateTest(it.second.plug)));
      PlugStore::getPlugStore().addPlug(it.second.package, it.second.name,
                                        it.second.runInternal, x, plug);
    }

    SamplerConfig sconfig(it.second); 
    InjectionPointStore::getInjectionPointStore().addInjectionPoint(
        it.second.package, it.second.name, it.second.runInternal, x, sconfig);
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



namespace {

// This is the last thing that needs to be implemented to get hotpatching running. 
// We need to implement something that ensures all processors (not just those in comm)
// have the same configuration in all cases.   
bool getHotPatchFileName(VnV_Comm comm, std::string& filename) { 

  VnV_Warn(VNVPACKAGENAME, "Hot Patching is not supported yet!");
  return false;
  
}



}  // namespace

void RunTime::loadHotPatch(VnV_Comm comm) {
  if (hotpatch) {
    // We need to think about this some. We should with relative ease be
    // able to support hot patching for injection points, plugs, iterations
    // unit tests and actions. Those are all just stored configurations. The
    // only requirement is that all procs get the same hot patch information.

    std::string hotpatchfilename;
    if (getHotPatchFileName(comm,hotpatchfilename)) {
      json j = JsonUtilities::load(hotpatchfilename);
      JsonParser parser;
      char** argv = nullptr;
      int argc = 0;

      RunInfo hinfo;

      try {
        hinfo = parser.parse(j, &argc, argv);
        loadRunInfo(hinfo, nullptr);
      } catch (VnVExceptionBase e) {
        std::cerr << "Loading of hot patch failed" << std::endl;
        std::cerr << e.what() << std::endl;
        return;
      }
    }
  }
}

// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv,
                           json& config, registrationCallBack* callback) {
  
  mainPackageName = packageName;


  JsonParser parser;
  try {
    info = parser.parse(config, argc, *argv);
  } catch (VnVExceptionBase e) {
    std::cerr << "VnV Initialization Failed during input file validation. \n";
    std::cerr << e.what() << std::endl;
    ;
    std::abort();
  }
  bool exitStatus = configure(packageName, info, callback);
  if (exitStatus != 0) {
    return true;
  }

  /**
   * Injection point documentation.
   **/

  VnV_Comm comm = CommunicationStore::instance().world();

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
      VNV_STR(VNVPACKAGENAME), comm, "configuration",
      [&](VnV_Comm comm, VnVParameterSet& p, OutputEngineManager* engine,
          InjectionPointType type, std::string stageId) {
        // caught everything, so internal ones can ignore parameters and put
        // anything.
        // Here, we do a minimal provenance history. For a full history, the
        // user should attach the provenace test to this node.
        auto config = p["config"].getByRtti<json>();
        auto argc = p["argc"].getByRtti<int*>();
        auto argv = p["argv"].getByRtti<char***>();

        std::string currTime = VnV::ProvenanceUtils::timeToString();
        std::string commandline =
            VnV::ProvenanceUtils::cmdLineToString(*argc, *argv);
        engine->Put("config", config);
        engine->Put("command-line", commandline);
        engine->Put("time", currTime);
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
  INJECTION_LOOP_BEGIN(VNV_STR(VNVPACKAGENAME), comm, "initialization",
                       runTests);

  return false;
}

bool RunTime::InitFromFile(const char* packageName, int* argc, char*** argv,
                           std::string configFile,
                           registrationCallBack* callback) {
  std::ifstream fstream(configFile);

  json mainJson = JsonUtilities::load(configFile);
  return InitFromJson(packageName, argc, argv, mainJson, callback);
}

bool RunTime::configure(std::string packageName, RunInfo info,
                        registrationCallBack* callback) {
  
  runTests = info.runTests;
  
  
  
  if (runTests) {
  
    CommunicationStore::instance().set(info.communicator);

    loadRunInfo(info, callback);

    if (info.unitTestInfo.runUnitTests) {
      runUnitTests(VnV_Comm_World(), info.unitTestInfo);

      if (info.unitTestInfo.exitAfterTests) {
        return 1;  // Exit because user asked to exit after unit tests
      }
    }

    runActions(VnV_Comm_World(), info.actionInfo,
               ActionType::configure());

  } else if (info.error) {
    return 2;  // Exit because there was an error.
  }

  return (getRunTimeOptions()->dumpConfig &&
          getRunTimeOptions()->exitAfterDumpConfig)
             ? 1
             : 0;
}

void RunTime::processToolConfig(json config, json& cmdline) {
  OptionsParserStore::instance().parse(config, cmdline);
}

void RunTime::runTimePackageRegistration(std::string packageName,
                                         registrationCallBack reg) {
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
    auto comm = CommunicationStore::instance().worldComm();

    INJECTION_LOOP_END(VNV_STR(VNVPACKAGENAME), "initialization");
    OutputEngineStore::getOutputEngineStore().getEngineManager()->finalize(
        comm);

    // Call any cleanup actions that were registered.
    for (auto& it : cleanupActions) {
      it.second(comm);
    }

    runActions(VWORLD, info.actionInfo, ActionType::finalize());
  }
  return true;
}

bool RunTime::isRunTests() { return runTests; }

void RunTime::log(VnV_Comm comm, std::string pname, std::string level,
                  std::string message, va_list args) {
  logger.log_c(comm, pname, level, message, args);
}

void RunTime::runUnitTests(VnV_Comm comm, UnitTestInfo info) {
  loadHotPatch(comm);
  UnitTestStore::getUnitTestStore().runAll(comm, info);
}

void RunTime::runActions(VnV_Comm comm, ActionInfo info, ActionType t) {
  // Load any hot patches.
  loadHotPatch(comm);
  ActionStore::getActionStore().runAll(comm, info, t);
}

void RunTime::readFile(std::string filename, long& idCounter) {
  OutputEngineStore::getOutputEngineStore().getEngineManager()->readFromFile(
      filename, idCounter);
}

void RunTime::printRunTimeInformation() {
  logger.print();
  OutputEngineStore::getOutputEngineStore().print();
  TestStore::instance().print();
  InjectionPointStore::getInjectionPointStore().print();
}
std::string RunTime::getPackageName() { return mainPackageName; }
