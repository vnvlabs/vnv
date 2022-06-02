﻿
/** @file Runtime.cpp Implementation of the Runtime class as defined in
 * base/Runtime.h"
 **/

#include "base/Runtime.h"

#include <unistd.h>

#include <iostream>

#include "base/DistUtils.h"
#include "base/Provenance.h"
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
#include "base/stores/WorkflowStore.h"
#include "c-interfaces/Logging.h"
#include "interfaces/IAction.h"
#include "interfaces/points/Injection.h"
#include "streaming/Nodes.h"

using namespace VnV;

/**
 * 
 * VnV Toolkit Version :vnv:`version[0]`
 * -------------------------------------
 * 
 * With access to high performance computational resources at an all time high, and with exascale computing resources on the horizon;
 * the role M&S has in the design pipelines of next generation technologies is only expected to increase. However, numerical simulations are,
 * by definition, an approximation to a real world physical system. As such, it is important that this increased reliance on simulated tests
 * is accompanied by a concerted effort to ensure simulations are fit for the intended purpose.
 *
 * The process of ensuring a code is fit for its intended purpose is called Verification and Validation (V&V) and generally includes:
 *   
 *   - Implementation of software development best practices (e.g. version control, unit and regression testing, code reviews, etc.).
 *   - Mathematical and algorithmic testing (convergence analysis, mesh refinement studies, method of manufactured solutions, etc.).
 *   - A broad benchmark testing suite including uncertainty quantification and sensitivity analysis.
 *   - Comparison of simulation results with experimental data and results from third party simulations. 
 *   - Review of the implementation and results by experts in the field.
 * 
 * V&V is a discrete process that cannot account for each and every possibility. This raises issues in the development of general purpose numerical
 * simulation packages because, while it is the simulation software developers responsibility to ensure the product is mathematically correct, 
 * it is ultimately the responsibility of the end user to ensure the solution is a suitable representation of their physical model. After all,
 * the direct costs of a design failure (be it time, money, or loss of life) fall squarely on the shoulders of the end-user; any attempt to 
 * shift the blame to the developers of simulation library *X* will certainly fall on deaf ears.
 * 
 * **VnV** was designed to facilitate end-user V&V in general
 * purpose numerical simulation packages. The framework will promote the development of **explainable numerical simulations**
 * that, in addition to the traditional simulation solution, produce a detailed report outlining how the solution was obtained and why 
 * it should be trusted. The VnV framework provides simple to maintain V&V methods for creating
 * self verifying, self describing, explainable numerical simulations.
 * 
 *    - **In-situ Testing And Analysis:** Unit tests are an effective mechanism for ensuring a function works as expected. 
 *      However, unit testing is an unavoidably discrete process that cannot cover every possible outcome. This is particularly
 *      true for numerical algorithms because even small changes (e.g., input parameters, mesh geometry, etc.) can cause the algorithms 
 *      to behave unexpectedly (i.e., diverge, converge to the wrong solution, etc.). As such, a robust V&V report should include a description
 *      of unit tests completed *and* a detailed set of tests and assertions that were completed during the simulation process.
 *      The VnV framework includes a sophisticated test injection system with cross-library support for defining testing points in existing codes.
 *      The framework will be able to configure injection points in any library linked to a simulation. For example,
 *      a MOOSE user would be able to run V&V tests at injection points defined in the source codes of hypre, PETSc, libMesh,
 *      MOOSE, etc through a single interface. This cross library support will allow for in-depth, expert directed, 
 *      end-user V&V in an executable that utilizes a range of numerical simulation libraries.
 *    
 *    - **Reusable Software Components:** While the specific details of V&V vary from application to application, 
 *      the macro scale algorithms are relatively consistent (e.g., mesh refinement studies, the method of manufactured solutions, 
 *      sensitivity analysis, uncertainty quantification and error propagation). Many of these algorithms can 
 *      be, or already have been, implemented as black-box or near black box solutions. The VnV framework will
 *      provide a robust set of near black-box tools that implement these common V&V approaches.
 *    
 *    - **Documentation Generation:** With software packages under almost constant development, 
 *      and new and improved packages being released on a regular basis, keeping an up-to-date V&V report 
 *      is an almost impossible task. The VnV toolkit will include automatic VnV report generation in the
 *      form of a server-less HTML web page. The report will be built using an extended markdown format 
 *      with support for standard markdown formatting, latex formatting, images, videos, self-sorting tables,
 *      two-dimensional charts, and three-dimensional visualization.
 *
 * 
 * .. vnv-image:: app/static/assets/images/VnVOut.png
 *    :width: 90%
 *    
 * 
 * 
 * In the image above, green boxes represent core functionalities. Developer interactions are shown in blue, 
 * runtime interactions are shown in orange and post-processing interactions are shown in black.
 * The figure above uses the MOOSE tool-chain to show how developers and end-users will interact with the VnV framework. 
 * The first step is to define the injection points. These injection points will be placed at key locations of the code 
 * where testing can and should take place. Developers will also complete an output template describing the state of
 * the simulation at each injection point. That specification will be used to populate the final VnV report.
 * 
 * The next step is to create a VnV test. The tests are developed in external libraries and hence, 
 * can be developed either by the developer of the simulation or by the end-user of the library. The core framework will also 
 * include a robust set of general purpose V&V tests. Each test will be accompanied by a markdown formatted template file.
 * Like injection points, this markdown file will be used to describe the test and present the results.
 * The VnV framework supports a custom markdown format that includes a range of data visualization techniques. We envision that
 * the developers of a numerical simulation package will ship the library with hard-coded injection points and a set of custom V&V tests.

 * End-users will be able to generate a customized input configuration file for each executable. This configuration file
 * will contain information about every injection point located in the call-graph of the simulation; 
 * including those in external third party libraries. After customizing that file, generating a VnV report is as simple as running the simulation.
 * 
 * Overall, once integrated into an application, the VnV framework will provide a simple mechanism for creating
 * self verifying, self describing, explainable numerical simulations. This will significantly reduce the burden
 * associated with V&V for end users, thereby increasing the usability of the tools for non-expert end-users. 
 * 
 */
INJECTION_OPTIONS(VNVPACKAGENAME, getBaseOptionsSchema().dump().c_str()) {
  RunTime::instance().getRunTimeOptions()->fromJson(config);
  engine->Put("version", 1.0);
  return NULL;
}

void RunTime::resetStore() { stores.clear(); }

namespace {

ICommunicator_ptr getComm(VnV_Comm comm) { return CommunicationStore::instance().getCommunicator(comm); }
}  // namespace

void RunTime::loadPlugin(std::string libraryPath, std::string packageName) {
  try {
    auto it = plugins.find(libraryPath);
    if (it == plugins.end()) {
      void* dllib = DistUtils::loadLibrary(libraryPath);
      if (dllib != nullptr) {

        registrationCallBack reg = DistUtils::searchLibrary(dllib, VNV_GET_REGISTRATION + packageName);
        if (reg != nullptr) {
          runTimePackageRegistration(packageName, reg);
        }
      } else {
        VnV_Warn(VNVPACKAGENAME, "Library not found");
      }
    }
  } catch (std::exception &e) {
    throw INJECTION_EXCEPTION("Error Loading Plugin: Library not found: %s", libraryPath.c_str());
  }
}

int RunTime::registerCleanUpAction(std::function<void(ICommunicator_ptr)> action) {
  int i = cleanupActionCounter++;
  cleanupActions[i] = action;
  return i;
}

void RunTime::makeLibraryRegistrationCallbacks(std::map<std::string, std::string> packageNames) {
  for (auto it : packageNames) {
    loadPlugin(it.second, it.first);
  }
}

bool RunTime::useAsciiColors() { return terminalSupportsAsciiColors; }

void RunTime::logUnhandled(std::string name, std::string id, NTV& args) {
  VnV_Info(VNVPACKAGENAME, "Name: %s", name.c_str());
  VnV_Info(VNVPACKAGENAME, "ID: %s", id.c_str());
  for (auto& it : args) {
    VnV_Info(VNVPACKAGENAME, "%s : (%s)", it.first.c_str(), it.second.first.c_str());
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

void dumpSpecification(bool quit) {
  std::cout << "===START SCHEMA DUMP===" << std::endl;
  std::cout << RunTime::instance().getFullJsonSchema().dump(4);
  std::cout << "===END SCHEMA_DUMP===" << std::endl;
  if (quit) {
    std::exit(0);
  }
}

nlohmann::json RunTime::getFullJsonSchema() {
  
  json packageJson = json::object();
  for (auto it : jsonCallbacks) {
    packageJson[it.first] =  json::parse((it.second)());
  }

  json main = json::parse(getVVSchema().dump());
  json& defs = main["definitions"];

  defs["options"] = OptionsParserStore::instance().schema(packageJson);

  defs["injectionPoints"] = InjectionPointStore::instance().schema(packageJson);
  defs["test"] = TestStore::instance().schema(packageJson);

  defs["iterator"] = IteratorStore::instance().schema(packageJson);
  defs["iteratorfunc"] = IteratorsStore::instance().schema(packageJson);

  defs["plug"] = PlugStore::instance().schema(packageJson);
  defs["plugger"] = PlugsStore::instance().schema(packageJson);

  defs["sampler"] = SamplerStore::instance().schema(packageJson);
  defs["transform"] = TransformStore::instance().schema(packageJson);

  defs["outputEngine"] = OutputEngineStore::instance().schema(packageJson);
  defs["communicator"] = CommunicationStore::instance().schema(packageJson);
  defs["actions"] = ActionStore::instance().schema(packageJson);
  defs["unittest"] = UnitTestStore::instance().schema(packageJson);
  defs["workflows"] = WorkflowStore::instance().schema(packageJson);
  
  json j = json::parse(jsonCallbacks[mainPackageName]());
  defs["executable"] = j["Executables"];
  return main;
}


nlohmann::json RunTime::getFullJson() {
  json main = json::object();

  for (auto& package : jsonCallbacks) {
    json j = json::parse(package.second());

    for (auto type : j.items()) {
      // Add all the options and stuff
      json& mj = JsonUtilities::getOrCreate(main, type.key(), JsonUtilities::CreateType::Object);

      if (type.key() == "Options") {
        json jf = OptionsParserStore::instance().getSchema(package.first);
        jf["docs"] = type.value();
        mj[package.first] = jf;
      } else if (type.key() == "DataType" || type.key() == "Files") {
        for (auto& entry : type.value().items()) {
          mj[package.first + ":" + entry.key()] = entry.value();
        }
      } else if (type.key() == "Introduction" || type.key() == "Conclusion") {
        if (package.first == mainPackageName) {
          mj["docs"] = type.value();
        }
      } else if (type.key().compare("InjectionPoints") == 0) {
        for (auto& entry : type.value().items()) {
          if (InjectionPointStore::instance().registered(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      } else if (type.key().compare("Tests") == 0) {
        for (auto& entry : type.value().items()) {
          if (InjectionPointStore::instance().registeredTest(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      } else if (type.key().compare("UnitTests") == 0) {
        if (info.unitTestInfo.runUnitTests) {
          for (auto& entry : type.value().items()) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      } else if (type.key().compare("Actions") == 0) {
        for (auto& entry : type.value().items()) {
          if (ActionStore::instance().registeredAction(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          } else {
          }
        }
      } else if (type.key().compare("Iterators") == 0) {
        for (auto& entry : type.value().items()) {
          if (IteratorStore::instance().registeredIterator(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      } else if (type.key().compare("Plugs") == 0) {
        for (auto& entry : type.value().items()) {
          if (PlugStore::instance().registeredPlug(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
        }
      } else if (type.key().compare("JobCreators") == 0) {
        for (auto& entry : type.value().items()) {
          if (WorkflowStore::instance().registeredJobCreator(package.first, entry.key())) {
            mj[package.first + ":" + entry.key()] = entry.value();
          }
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
  VnV_Iterator_Info(std::string n, std::string p, int o, std::string f, int l, std::shared_ptr<IterationPoint>& it)
      : name(n), pname(p), once(o), fname(f), line(l), iter(it) {}
};

}  // namespace

std::shared_ptr<IterationPoint> RunTime::getNewInjectionIteration(VnV_Comm comm, std::string pname, std::string id,
                                                                  struct VnV_Function_Sig pretty,
                                                                  InjectionPointType type, int once, NTV& args) {
  if (runTests) {
    // Load any hot patches
    loadHotPatch(comm);

    std::shared_ptr<IterationPoint> ipd = IteratorStore::instance().getNewIterator(pname, id, pretty, once, args);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, args);
    }
  }
  return nullptr;
}

VnV_Iterator RunTime::injectionIteration(VnV_Comm comm, std::string pname, std::string id,
                                         struct VnV_Function_Sig pretty, std::string fname, int line,
                                         const DataCallback& callback, NTV& args, int once) {
  try {
    auto engine = OutputEngineStore::instance().getEngineManager();

    ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

    auto it = getNewInjectionIteration(comm, pname, id, pretty, InjectionPointType::Begin, once, args);
    if (it != nullptr) {
      it->setComm(getComm(comm));
      it->setCallBack(callback);
      it->iterate(fname, line);
    }

    VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, fname, line, it);
    return {(void*)info};
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Injection iteration failed %s:%s", pname.c_str(), id.c_str());
    std::shared_ptr<VnV::IterationPoint> s;
    VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, fname, line, s);
    return {(void*)info};
  }
}

VnV_Iterator RunTime::injectionIteration(VnV_Comm comm, std::string pname, std::string id,
                                         struct VnV_Function_Sig pretty, std::string fname, int line,
                                         injectionDataCallback* callback, NTV& args, int once) {
  try {
    ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

    auto it = getNewInjectionIteration(comm, pname, id, pretty, InjectionPointType::Begin, once, args);
    if (it != nullptr) {
      it->setComm(getComm(comm));
      it->setCallBack(callback);
      it->iterate(fname, line);
    }

    VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, fname, line, it);

    return {(void*)info};

  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Injection iteration failed %s:%s", pname.c_str(), id.c_str());
    std::shared_ptr<VnV::IterationPoint> s;
    VnV_Iterator_Info* info = new VnV_Iterator_Info(id, pname, once, fname, line, s);
    return {(void*)info};
  }
}

int RunTime::injectionIterationRun(VnV_Iterator* iterator) {
  VnV_Iterator_Info* info = (VnV_Iterator_Info*)iterator->data;

  ActionStore::instance().injectionPointIter(std::to_string(info->count));

  if (info->iter != nullptr) {
    bool a = false;
    try {
      a = info->iter->iterate(info->fname, info->line);
      if (!a) {
        ActionStore::instance().injectionPointEnd();
      }
      return a;
    } catch (std::exception& e) {
      VnV_Error(VNVPACKAGENAME, "Error Occured during iteration : %s", e.what());
    } 
  }

  if (info->count < info->once) {
    info->count++;
    return 1;
  } else {
    ActionStore::instance().injectionPointEnd();
    return 0;
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
  VnV_Plug_Info(std::string n, std::string p, std::string f, int l, std::shared_ptr<PlugPoint>& it)
      : name(n), pname(p), function(f), line(l), iter(it) {}
};
}  // namespace

std::shared_ptr<PlugPoint> RunTime::getNewInjectionPlug(VnV_Comm comm, std::string pname, std::string id,
                                                        struct VnV_Function_Sig pretty, NTV& args) {
  if (runTests) {
    // load hotpatches
    loadHotPatch(comm);

    std::shared_ptr<PlugPoint> ipd = PlugStore::instance().getNewPlug(pname, id, pretty, args);
    if (ipd != nullptr) {
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, args);
    }
  }
  return nullptr;
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                    std::string fname, int line, const DataCallback& callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  std::shared_ptr<VnV::PlugPoint> it;
  try {
    auto it = getNewInjectionPlug(comm, pname, id, pretty, args);
    if (it != nullptr) {
      it->setComm(getComm(comm));
      it->setCallBack(callback);
    }
  } catch (std::exception &e) {
    it = nullptr;
  }

  VnV_Plug_Info* info = new VnV_Plug_Info(id, pname, fname, line, it);
  return {(void*)info};
}

VnV_Iterator RunTime::injectionPlug(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                    std::string fname, int line, injectionDataCallback* callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  std::shared_ptr<VnV::PlugPoint> it;
  try {
    it = getNewInjectionPlug(comm, pname, id, pretty, args);
    if (it != nullptr) {
      it->setComm(getComm(comm));
      it->setCallBack(callback);
    }
  } catch (std::exception &e) {
    it = nullptr;
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

std::shared_ptr<InjectionPoint> RunTime::getNewInjectionPoint(VnV_Comm comm, std::string pname, std::string id,
                                                              struct VnV_Function_Sig pretty, InjectionPointType type,
                                                              NTV& args) {
  if (runTests) {
    // look for hotpatches;
    loadHotPatch(comm);

    std::shared_ptr<InjectionPoint> ipd =
        InjectionPointStore::instance().getNewInjectionPoint(pname, id, pretty, type, args);
    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, "Begin");
      return ipd;
    } else if (runTimeOptions.logUnhandled) {
      logUnhandled(pname, id, args);
    }
  }
  return nullptr;
}

std::shared_ptr<InjectionPoint> RunTime::getExistingInjectionPoint(std::string pname, std::string id,
                                                                   InjectionPointType type, std::string stageId) {
  if (runTests) {
    std::shared_ptr<InjectionPoint> ipd = InjectionPointStore::instance().getExistingInjectionPoint(pname, id, type);

    if (ipd != nullptr) {
      ipd->setInjectionPointType(type, stageId);
      return ipd;
    }
  }

  return nullptr;
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                   std::string fname, int line, const DataCallback& callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  auto it = getNewInjectionPoint(comm, pname, id, pretty, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }
}

void RunTime::injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                   std::string fname, int line, injectionDataCallback* callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  auto it = getNewInjectionPoint(comm, pname, id, pretty, InjectionPointType::Begin, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }
}

// Cpp interface.
void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                             std::string fname, int line, const DataCallback& callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  auto it = getNewInjectionPoint(comm, pname, id, pretty, InjectionPointType::Single, args);
  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }

  ActionStore::instance().injectionPointEnd();
}

void RunTime::injectionPoint(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                             std::string fname, int line, injectionDataCallback* callback, NTV& args) {
  ActionStore::instance().injectionPointStart(getComm(comm), pname, id);

  auto it = getNewInjectionPoint(comm, pname, id, pretty, InjectionPointType::Single, args);

  if (it != nullptr) {
    it->setCallBack(callback);
    it->setComm(getComm(comm));
    it->run(fname, line);
  }

  ActionStore::instance().injectionPointEnd();
}

void RunTime::injectionPoint_iter(std::string pname, std::string id, std::string stageId, std::string fname, int line) {
  ActionStore::instance().injectionPointIter(stageId);

  auto it = getExistingInjectionPoint(pname, id, InjectionPointType::Iter, stageId);
  if (it != nullptr) {
    it->run(fname, line);
  }
}

void RunTime::injectionPoint_end(std::string pname, std::string id, std::string fname, int line) {
  ActionStore::instance().injectionPointEnd();

  auto it = getExistingInjectionPoint(pname, id, InjectionPointType::End, "End");
  if (it != nullptr) {
    it->run(fname, line);
  }
}

void RunTime::declarePackageJson(std::string pname, vnvFullJsonStrCallback callback) {
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

RunTime::RunTime() {
  // Set the workflow name and job ids.
  workflowName_ = DistUtils::getEnvironmentVariable("VNV_WORKFLOW_ID", StringUtils::random(5));
  workflowJob_ = StringUtils::random(5);
  start = std::chrono::steady_clock::now();
}

void RunTime::registerLogLevel(std::string packageName, std::string name, std::string color) {
  logger.registerLogLevel(packageName, name, color);
}

void RunTime::registerFile(VnV_Comm comm, std::string packageName, std::string name, int input, std::string filename,
                           std::string reader) {
  std::string fname = DistUtils::getAbsolutePath(filename);
  OutputEngineStore::instance().getEngineManager()->file(getComm(comm), packageName, name, input, fname, reader);
}

void RunTimeOptions::callback(json& j) { RunTime::instance().runTimeOptions.fromJson(j); }

void RunTimeOptions::fromJson(json& j) {
  if (j.contains("logUnhandled")) {
    logUnhandled = j["logUnhandled"].get<bool>();
  }
}

void RunTime::getFullSchema(std::string filename) {}

void RunTime::writeRunInfoFile() {
  // WorkflowName is fixed for for a particular "Job"
  // Workflow Job is unique to this example.

  std::string f = "vnv_" + workflowName() + "_" + workflowJob() + ".runInfo";

  std::string filename = DistUtils::join({workflowDir_, f}, 0777, false);
  std::ofstream ofs(filename);
  if (ofs.good()) {
    json rinfo = json::object();
    rinfo["workflow"] = workflowName();
    rinfo["name"] = workflowJob();
    rinfo["engine"] = OutputEngineStore::instance().getRunInfo();
    rinfo["alias"] = DistUtils::getEnvironmentVariable("VNV_RUN_ALIAS", workflowJob());
    ofs << rinfo.dump(3);
    ofs.close();
  }
}

void RunTime::loadRunInfo(RunInfo& info, registrationCallBack callback) {
  initializedCount++;

  if (initializedCount == 1) {
    // Set up the logger. This occurs as early as possible to allow log messages
    // to be caught int the registration objects.
    if (info.logInfo.on) {
      
      logger.setLog(info.logInfo.engine,info.logInfo.type,info.logInfo.filename);
      

      for (auto it : info.logInfo.logs) {
        logger.setLogLevel(it.first, it.second);
      }
      for (auto it : info.logInfo.blackList) {
        logger.addToBlackList(it);
      }
    }

    if (!info.workflowDir.empty()) {
      workflowDir_ = info.workflowDir;
    }

    // Pull out the template patch. This is the user provides specification file
    // that should be merged into the final specificiation.
    template_patch = info.template_overrides;

    // Register VnV.
    runTimePackageRegistration(VNV_STR(VNVPACKAGENAME), INJECTION_REGISTRATION_PTR(VNVPACKAGENAME));

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
  logger.setRank(world->Rank());

  if (initializedCount == 1) {
    if (!OutputEngineStore::instance().isInitialized()) {
      VnV_Debug(VNVPACKAGENAME, "Configuring The Output Engine");
      try {
        OutputEngineStore::instance().setEngineManager(world, info.engineInfo.engineType, info.engineInfo.engineConfig);

      } catch (VnVExceptionBase& e) {
        VnV_Error(VNVPACKAGENAME,
                  "Error Initializing Engine: What happens next will depend on the 'onEngineInitializationFailed' "
                  "parameter.");

        // TODO Allow user to decide what happens here.
        VnV_Error(VNVPACKAGENAME, "Aborting");
        std::abort();
      }
      VnV_Debug(VNVPACKAGENAME, "Output Engine Configuration Successful");
    }
  }

  // Process the configs (wait until now because it allows loaded test libraries
  // to register options objects.
  processToolConfig(info.pluginConfig, info.cmdline, world);

  if (info.schemaDump) {
    dumpSpecification(info.schemaQuit);
  }

  // Write the run info file -- This contains all the info needed to launch the reader.
  writeRunInfoFile();

  hotpatch = info.hotpatch;

  VnV_Debug(VNVPACKAGENAME,
            "Validating Json Test Configuration Input and converting to TestConfig "
            "objects");

  for (auto it : info.injectionPoints) {
    auto x = TestStore::instance().validateTests(it.second.tests);

    if (it.second.type == InjectionType::ITER) {
      auto iterations = IteratorsStore::instance().validateTests(it.second.iterators);
      IteratorStore::instance().addIterator(it.second.package, it.second.name, it.second.runInternal,
                                            it.second.templateName, x, iterations);

    } else if (it.second.type == InjectionType::PLUG) {
      try {
        auto plug = std::make_shared<PlugConfig>(std::move(PlugsStore::instance().validateTest(it.second.plug)));
        PlugStore::instance().addPlug(it.second.package, it.second.name, it.second.runInternal, it.second.templateName,
                                      x, plug);
      } catch (VnVExceptionBase& e) {
        VnV_Error(VNVPACKAGENAME, "Failed to add plug %s:%s --- %s", it.second.package.c_str(), it.second.name.c_str(),
                  e.what());
      }
    } else if (it.second.type == InjectionType::POINT) {
      SamplerConfig sconfig(it.second);
      InjectionPointStore::instance().addInjectionPoint(it.second.package, it.second.name, it.second.runInternal,
                                                        it.second.templateName, x, sconfig);
    } else {
      throw INJECTION_BUG_REPORT("Unknown Injection point type %d", it.second.type);
    }
  }
  jobManager = WorkflowStore::instance().buildJobManager(mainPackageName, workflowName(), info.workflowInfo.workflows);

  OutputEngineStore::instance().getEngineManager()->sendInfoNode(world);
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
bool RunTime::InitFromJson(const char* packageName, int* argc, char*** argv, json& config,
                           registrationCallBack callback) {
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
      dumpSpecification(false);
      break;
    } else if (s.compare("--vnv-qdump") == 0) {
      dumpSpecification(false);
      break;
    } else if (s.compare("--vnv-fdump") == 0) {
      std::string dumpfile = (*argv)[i + 1];
      writeSpecification(dumpfile, false);
      break;
    } else if (s.compare("--vnv-qfdump") == 0) {
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
   * @title VnV Application Profiling Loop.
   * @description Initialization of the VnV Loop   
   * @instructions Use this to track updates for the entire application.
   * @param runTests A bool indicating if run Tests is turned on
   *
   * This injection point is called at the end of the VnVInit function. This is a
   * looped injection point with no interesting parameters passed in. This
   * injection point exists soley as a mechanism for profiling the given
   * application between the VnVInit and VnVFinalize functions.
   *
   */
  INJECTION_LOOP_BEGIN(VNVPACKAGENAME, comm, initialization, runTests);

  return false;
}

bool RunTime::InitFromFile(const char* packageName, int* argc, char*** argv, std::string configFile,
                           registrationCallBack callback) {
  
  
  std::string fname = DistUtils::getEnvironmentVariable("VNV_INPUT_FILE","");
  if (fname.length() > 0 ) {
    std::cout << "Using Input file defined in the environment: " << fname;
    configFile = fname;
  } else {

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

  }

  this->configFile = configFile;
  if (configFile == VNV_DEFAULT_INPUT_FILE) {
    json j = json::object();
    return InitFromJson(packageName, argc, argv, j, callback);
  }

  std::ifstream fstream(configFile);
  if (fstream.good()) {
    json mainJson = JsonUtilities::load(configFile);
    return InitFromJson(packageName, argc, argv, mainJson, callback);
  }
  throw INJECTION_EXCEPTION("Bad Input File %s", configFile.c_str());
}

bool RunTime::configure(std::string packageName, RunInfo info, registrationCallBack callback) {
  runTests = info.runTests;

  if (runTests) {
    CommunicationStore::instance().set(info.communicator);
    auto commW = CommunicationStore::instance().worldComm();

    loadRunInfo(info, callback);

    // Run any workflows listed in the application
    jobManager->run(commW, true);

    if (info.unitTestInfo.runUnitTests) {
      runUnitTests(VnV_Comm_World(), info.unitTestInfo);

      if (info.unitTestInfo.exitAfterTests) {
        return 1;  // Exit because user asked to exit after unit tests
      }
    }

    ActionStore::instance().initialize(commW);

  } else if (info.error) {
    return 2;  // Exit because there was an error.
  }

  return 0;
}

void RunTime::processToolConfig(json config, json& cmdline, ICommunicator_ptr world) {
  OptionsParserStore::instance().parse(config, cmdline, world);
}

void RunTime::runTimePackageRegistration(std::string packageName, registrationCallBack reg) {
  auto it = plugins.find(packageName);
  if (it == plugins.end()) {
    VnV_Debug(VNVPACKAGENAME, "Registering a new Package %s", packageName.c_str());
    plugins.insert(packageName);
    reg();  // could be recursive.
  }
}

long RunTime::currentTime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}

bool RunTime::Finalize() {
  if (runTests) {
    auto comm = CommunicationStore::instance().worldComm();

    INJECTION_LOOP_END(VNVPACKAGENAME, initialization);

    ActionStore::instance().finalize(comm);

    // Run any workflow jobs marked for execution after the workflow finishs.,
    jobManager->run(comm, false);

    auto engine = OutputEngineStore::instance().getEngineManager();
    engine->finalize(comm, currentTime());

    // Call any cleanup actions that were registered.
    for (auto& it : cleanupActions) {
      it.second(comm);
    }
  }

  resetStore();
  return true;
}

bool RunTime::isRunTests() { return runTests; }

void RunTime::log(VnV_Comm comm, std::string pname, std::string level, std::string message, va_list args) {
  logger.log_c(comm, pname, level, message, args);
}
void RunTime::log(VnV_Comm comm, std::string pname, std::string level, std::string message) {
  logger.log(comm, pname, level, message);
}

void RunTime::runUnitTests(VnV_Comm comm, UnitTestInfo info) {
  loadHotPatch(comm);
  UnitTestStore::instance().runAll(comm, info);
}

std::shared_ptr<Nodes::IRootNode> RunTime::readFile(std::string reader, std::string filename) {
  json j = json::object();
  return OutputEngineStore::instance().readFile(filename, reader, j);
}

void RunTime::readFileAndWalk(std::string reader, std::string filename, std::string pack, std::string walk,
                              nlohmann::json con) {
  auto rootNode = readFile(reader, filename);

  std::shared_ptr<IWalker> walker = WalkerStore::instance().getWalker(pack, walk, rootNode.get(), con);
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

void* RunTime::getOptionsObject(std::string package) {
  return OptionsParserStore::instance().getResult(package);
}
  