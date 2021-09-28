
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include <unistd.h>
#include <iostream>

#include "base/DistUtils.h"
#include "base/Provenance.h"
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
#include "base/stores/SamplerStore.h"
#include "base/stores/TestStore.h"
#include "base/stores/UnitTestStore.h"
#include "base/stores/WalkerStore.h"
#include "c-interfaces/Logging.h"
#include "interfaces/IAction.h"
#include "interfaces/points/Injection.h"

using namespace VnV;

INJECTION_OPTIONS(VNVPACKAGENAME, getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
}

void RunTime::resetStore() { stores.clear(); }

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

void RunTime::writeSpecification(std::string filename, bool quit) {
  std::ofstream ofs(filename);
  if (ofs.good()) {
    ofs << getFullJsonSchema().dump(4);
  }
  if (quit) {
    std::exit(0);
  }
}

nlohmann::json RunTime::getFullJsonSchema() {
  json main = json::parse(getVVSchema().dump());
  main["options"] = OptionsParserStore::instance().schema();
  json& defs = main["definitions"];
  defs["injectionPoint"] = InjectionPointStore::instance().schema();
  defs["iterator"] = IteratorStore::instance().schema();
  defs["plug"] = PlugStore::instance().schema();
  defs["test"] = TestStore::instance().schema();
  defs["iteratorfunc"] = IteratorsStore::instance().schema();
  defs["plugger"] = PlugsStore::instance().schema();
  defs["sampler"] = SamplerStore::instance().schema();
  defs["outputEngine"] = OutputEngineStore::instance().schema();
  defs["communicator"] = CommunicationStore::instance().schema();
  defs["action"] = ActionStore::instance().schema();
  defs["unittest"] = UnitTestStore::instance().schema();
  defs["transform"] = TransformStore::instance().schema();
  return main;
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
  std::string fname;
  int line;
  int once;
  int count = 0;
  std::shared_ptr<IterationPoint> iter;
  VnV_Iterator_Info(std::string n, std::string p, int o, std::string f, int l,
                    std::shared_ptr<IterationPoint>& it)
      : name(n), pname(p), once(o), fname(f), line(l), iter(it) {}
};

}  // namespace

std::shared_ptr<IterationPoint> RunTime::getNewInjectionIteration(
    VnV_Comm comm, std::string pname, std::string id, InjectionPointType type,
    int once, NTV& in_args, NTV& out_args) {
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
                                         std::string id, std::string fname,
                                         int line, const DataCallback& callback,
                                         NTV& inputs, NTV& outputs, int once) {
  
  
  auto engine = OutputEngineStore::instance().getEngineManager();
  
  ActionStore::instance().injectionPointStart(getComm(comm),pname, id);
  
  auto it = getNewInjectionIteration(comm, pname, id, InjectionPointType::Begin,
                                     once, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
    it->iterate(fname, line);
  }
    
  VnV_Iterator_Info* info =
      new VnV_Iterator_Info(id, pname, once, fname, line, it);
  return {(void*)info};
}

VnV_Iterator RunTime::injectionIteration(VnV_Comm comm, std::string pname,
                                         std::string id, std::string fname,
                                         int line,
                                         injectionDataCallback* callback,
                                         NTV& inputs, NTV& outputs, int once) {
  
  ActionStore::instance().injectionPointStart(getComm(comm),pname, id);
  
  
  auto it = getNewInjectionIteration(comm, pname, id, InjectionPointType::Begin,
                                     once, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
    it->iterate(fname, line);
  }
  
  VnV_Iterator_Info* info =
      new VnV_Iterator_Info(id, pname, once, fname, line, it);
  
  return {(void*)info};
}

int RunTime::injectionIterationRun(VnV_Iterator* iterator) {

  VnV_Iterator_Info* info = (VnV_Iterator_Info*)iterator->data;

  ActionStore::instance().injectionPointIter(std::to_string(info->count));

  if (info->iter != nullptr) {
    auto a = info->iter->iterate(info->fname, info->line);
    if (!a) {
      ActionStore::instance().injectionPointEnd();
    }
    return a;
  } else {
    if (info->count < info->once) {
      info->count++;
      return 1;
    } else {
      ActionStore::instance().injectionPointEnd();
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
  std::string function;
  int line;
  std::shared_ptr<PlugPoint> iter;
  VnV_Plug_Info(std::string n, std::string p, std::string f, int l,
                std::shared_ptr<PlugPoint>& it)
      : name(n), pname(p), function(f), line(l), iter(it) {}
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
        PlugStore::instance().getNewPlug(pname, id, in_args, out_args);
    if (ipd != nullptr) {
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, in_args);
    }
  }
  return nullptr;
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname,
                                    std::string id, std::string fname, int line,
                                    const DataCallback& callback, NTV& inputs,
                                    NTV& outputs) {
  
  ActionStore::instance().injectionPointStart(getComm(comm),pname, id);
    
  auto it = getNewInjectionPlug(comm, pname, id, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
  }
  VnV_Plug_Info* info = new VnV_Plug_Info(id, pname, fname, line, it);
  return {(void*)info};
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname,
                                    std::string id, std::string fname, int line,
                                    injectionDataCallback* callback,
                                    NTV& inputs, NTV& outputs) {
  
  ActionStore::instance().injectionPointStart(getComm(comm),pname, id);
  

  auto it = getNewInjectionPlug(comm, pname, id, inputs, outputs);
  if (it != nullptr) {
    it->setComm(getComm(comm));
    it->setCallBack(callback);
  }
  VnV_Plug_Info* info = new VnV_Plug_Info(id, pname, fname, line, it);
  return {(void*)info};
}

int RunTime::injectionPlugRun(VnV_Iterator* iterator) {
  VnV_Plug_Info* info = (VnV_Plug_Info*)iterator->data;
  ActionStore::instance().injectionPointIter("Plug");
  if (info->iter != nullptr) {
    info->iter->plug(info->function, info->line);
    ActionStore::instance().injectionPointEnd();
    return 1;
  } else {
    ActionStore::instance().injectionPointEnd();
    return 0;
  }
}

/************************** INJECTION POINTS
 * ****************************************/

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(
    VnV_Comm comm, std::string pname, std::string id, InjectionPointType type,
    NTV& in_args) {
  if (runTests) {
    // look for hotpatches;
    loadHotPatch(comm);

    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::instance().getNewInjectionPoint(pname, id, type,
                                                             in_args);
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
        InjectionPointStore::instance().getExistingInjectionPoint(pname, id,
                                                                  type);

    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, stageId);
      return ipd;
    }
  }

  return nullptr;
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id, std::string fname, int line,
                                   const DataCallback& callback, NTV& args) {
  
  
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);
  
  auto it =
      getNewInjectionPoint(comm, pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname,
                                   std::string id, std::string fname, int line,
                                   injectionDataCallback* callback, NTV& args) {
  
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);
  
  auto it =
      getNewInjectionPoint(comm, pname, id, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }
}

// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             std::string fname, int line,
                             const DataCallback& callback, NTV& args) {
  
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  
  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }

  ActionStore::instance().injectionPointEnd();

}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                             std::string fname, int line,
                             injectionDataCallback* callback, NTV& args) {
  
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  auto it = getNewInjectionPoint(comm, pname, id, InjectionPointType::Single, args);
 
 
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }

  ActionStore::instance().injectionPointEnd();

}

void RunTime::injectionPoint_iter(std::string pname, std::string id,
                                  std::string stageId, std::string fname,
                                  int line) {
  
  ActionStore::instance().injectionPointIter(stageId);
  
  auto it =
      getExistingInjectionPoint(pname, id, InjectionPointType::Iter, stageId);
  if (it != nullptr) {
    it->run(fname, line);
  }

}

void RunTime::injectionPoint_end(std::string pname, std::string id,
                                 std::string fname, int line) {

  ActionStore::instance().injectionPointEnd();
   
  auto it = getExistingInjectionPoint(pname, id, InjectionPointType::End, "End");
  if (it != nullptr) {
    it->run(fname, line);
  }

 
}

void RunTime::declarePackageJson(std::string pname,
                                 vnvFullJsonStrCallback callback) {
  jsonCallbacks.insert(std::make_pair(pname, callback));
}

RunTime& RunTime::instance(bool reset) {
  static RunTime store;
  if (reset) {
    store = RunTime();
  }
  return store;
}
RunTime& RunTime::instance() { return instance(false); }
RunTime& RunTime::reset() { return instance(true); }

RunTime::RunTime() { start = std::chrono::steady_clock::now(); }

void RunTime::registerLogLevel(std::string packageName, std::string name,
                               std::string color) {
  logger.registerLogLevel(packageName, name, color);
}

void RunTime::registerFile(VnV_Comm comm, std::string packageName,
                           std::string name, int input, std::string filename,
                           std::string reader) {
  std::string fname = DistUtils::getAbsolutePath(filename);
  OutputEngineStore::instance().getEngineManager()->file(
      getComm(comm), packageName, name, input, fname, reader);
}

void RunTimeOptions::callback(json& j) {
  RunTime::instance().runTimeOptions.fromJson(j);
}

void RunTimeOptions::fromJson(json& j) {
  if (j.contains("logUnhandled")) {
    logUnhandled = j["logUnhandled"].get<bool>();
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

    // Register VnV.
    runTimePackageRegistration(VNV_STR(VNVPACKAGENAME),
                               INJECTION_REGISTRATION_PTR(VNVPACKAGENAME));

    // Register the Executable.
    if (callback != nullptr) {
      runTimePackageRegistration(mainPackageName, *callback);
    }

    // Now we register VnVs communicator. The VnV communicator is set to be the
    // same as the communicator for the executable.
    // CommunicationStore::instance().copySettings(mainPackageName,VNVPACKAGENAME_S);
  }

  // Register the plugins specified in the input file.
  makeLibraryRegistrationCallbacks(info.additionalPlugins);

  // Set up the engine
  ICommunicator_ptr world = CommunicationStore::instance().worldComm();

  if (initializedCount == 1) {
    if (!OutputEngineStore::instance().isInitialized()) {
      VnV_Debug(VNVPACKAGENAME, "Configuring The Output Engine");
      OutputEngineStore::instance().setEngineManager(
          world, info.engineInfo.engineType, info.engineInfo.engineConfig);
      VnV_Debug(VNVPACKAGENAME, "Output Engine Configuration Successful");
    }
  }

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.pluginConfig, info.cmdline, world);


  hotpatch = info.hotpatch;

  VnV_Debug(
      VNVPACKAGENAME,
      "Validating Json Test Configuration Input and converting to TestConfig "
      "objects");

  for (auto it : info.injectionPoints) {
    auto x = TestStore::instance().validateTests(it.second.tests);

    if (it.second.type == InjectionType::ITER) {
      auto iterations =
          IteratorsStore::instance().validateTests(it.second.iterators);
      IteratorStore::instance().addIterator(it.second.package, it.second.name,
                                            it.second.runInternal, x,
                                            iterations);

    } else if (it.second.type == InjectionType::PLUG) {
      auto plug = std::make_shared<PlugConfig>(
          std::move(PlugsStore::instance().validateTest(it.second.plug)));
      PlugStore::instance().addPlug(it.second.package, it.second.name,
                                    it.second.runInternal, x, plug);
    }

    SamplerConfig sconfig(it.second);
    InjectionPointStore::instance().addInjectionPoint(
        it.second.package, it.second.name, it.second.runInternal, x, sconfig);
  }

  ActionStore::instance().initialize(info.actionInfo);


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

// This is the last thing that needs to be implemented to get hotpatching
// running. We need to implement something that ensures all processors (not just
// those in comm) have the same configuration in all cases.
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
    if (getHotPatchFileName(comm, hotpatchfilename)) {
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

VnVProv RunTime::getProv() { return *prov; }

// Cant overload the name because "json" can be a "string".
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv,
                           json& config, registrationCallBack* callback) {
  mainPackageName = packageName;

  // Set the provenance information .
  prov.reset(new VnV::VnVProv(*argc, *argv, configFile, config));

  JsonParser parser;
  try {
    info = parser.parse(config, argc, *argv);
  } catch (VnVExceptionBase e) {
    std::cerr << "VnV Initialization Failed during input file validation. \n";
    std::cerr << e.what() << std::endl;
    std::abort();
  }

  bool exitStatus = configure(packageName, info, callback);
  if (exitStatus != 0) {
    return true;
  }

  // Search the Cmd line for the input override call. This call sets the
  // configuration file using the command line. It overrides the value passed in
  // through the initialize call.
  for (int i = 0; i < *argc; i++) {
    std::string s((*argv)[i]);
    if (s.compare("--vnv-dump") == 0) {
      std::string dumpfile = (*argv)[i + 1];
      writeSpecification(dumpfile, false);
      break;
    } else if (s.compare("--vnv-qdump") == 0) {
      std::string dumpfile = (*argv)[i + 1];
      writeSpecification(dumpfile, true);
      std::exit(0);
    }
  }

  prov.reset(new VnV::VnVProv(*argc, *argv, configFile, config));

  /**
   * Injection point documentation.
   **/

  VnV_Comm comm = CommunicationStore::instance().world();
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
  // Search the Cmd line for the input override call. This call sets the
  // configuration file using the command line. It overrides the value passed in
  // through the initialize call.
  for (int i = 0; i < *argc; i++) {
    std::string s((*argv)[i]);
    if (s.compare("--vnv-input-file") == 0) {
      configFile = (*argv)[i + 1];
      break;
    }
  }

  this->configFile = configFile;
  if (configFile == VNV_DEFAULT_INPUT_FILE) {
    json j = json::object();
    return InitFromJson(packageName, argc, argv, j, callback);
  }

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

    auto commW = CommunicationStore::instance().worldComm();
    ActionStore::instance().initialize(commW);

  } else if (info.error) {
    return 2;  // Exit because there was an error.
  }

  return 0;
}

void RunTime::processToolConfig(json config, json& cmdline,
                                ICommunicator_ptr world) {
  OptionsParserStore::instance().parse(config, cmdline, world);
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

long RunTime::duration() {
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
      .count();
}

bool RunTime::Finalize() {
  if (runTests) {
    auto comm = CommunicationStore::instance().worldComm();

    INJECTION_LOOP_END(VNV_STR(VNVPACKAGENAME), "initialization");

    ActionStore::instance().finalize(comm);
  
    auto engine = OutputEngineStore::instance().getEngineManager();
    engine->finalize(comm, duration());

    // Call any cleanup actions that were registered.
    for (auto& it : cleanupActions) {
      it.second(comm);
    }

  }  

  resetStore();
  return true;
}

bool RunTime::isRunTests() { return runTests; }

void RunTime::log(VnV_Comm comm, std::string pname, std::string level,
                  std::string message, va_list args) {
  logger.log_c(comm, pname, level, message, args);
}

void RunTime::runUnitTests(VnV_Comm comm, UnitTestInfo info) {
  loadHotPatch(comm);
  UnitTestStore::instance().runAll(comm, info);
}


std::shared_ptr<Nodes::IRootNode> RunTime::readFile(std::string reader,
                                                    std::string filename) {
  json j = json::object();
  return OutputEngineStore::instance().readFile(filename, reader, j);
}

void RunTime::readFileAndWalk(std::string reader, std::string filename,
                              std::string pack, std::string walk,
                              nlohmann::json con) {
  auto rootNode = readFile(reader, filename);

  std::shared_ptr<IWalker> walker =
      WalkerStore::instance().getWalker(pack, walk, rootNode.get(), con);
  VnV::Nodes::WalkerNode node;

  while (walker->next(node)) {
    std::cout << node.item->getName() << std::endl;
  }

}

void RunTime::printRunTimeInformation() {
  logger.print();
  OutputEngineStore::instance().print();
  TestStore::instance().print();
  InjectionPointStore::instance().print();
}
std::string RunTime::getPackageName() { return mainPackageName; }
