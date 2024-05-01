
/**
  @file Runtime.h
**/

#ifndef VV_RUNTIME_HEADER
#define VV_RUNTIME_HEADER

/**
  \file VV-Runtime class.
  */
#include <stdarg.h>

#include <chrono>
#include <json-schema.hpp>
#include <map>
#include <string>
#include <typeindex>

#include "base/Logger.h"
#include "base/parser/JsonParser.h"
#include "base/points/InjectionPoint.h"
#include "base/points/IteratorPoint.h"
#include "base/points/PlugPoint.h"
#include "base/stores/BaseStore.h"
#include "c-interfaces/Wrappers.h"
#include "common-interfaces/RunTime.h"
#include "interfaces/Initialization.h"

/**
 * VnV Namespace
 */

using nlohmann::json;

namespace VnV {

class VnVProv;

/**
 * @brief The RunTime clas
 *
 * The runtime class is the (semi) user facing class of the library. This class
 * handles running of injection points in libraries, as well as initialization
 * and finalization of the VnV Objects and Engines.
 *
 * Like the EngineStore, the RunTime class uses a static initialization function
 * and a private constructor. This makes it impossible to create more than one
 * instance of the RunTime Class.
 *
 */

class JobManager;

class RunTimeOptions {
 public:
  bool logUnhandled = false;
  void fromJson(json& jsonObject);
  static void callback(json& j);
};

class RunTime {
  friend class RunTimeOptions;

 private:
  Logger logger;
  std::chrono::_V2::steady_clock::time_point start;

  /**
   * @brief RunTime
   *
   * Private Constructor -- Called used the getRunTime() function.
   *
   */
  RunTime();

  std::map<int, std::function<void(ICommunicator_ptr)>> cleanupActions;

  std::string mainPackageName;
  std::set<std::string> plugins;
  std::set<std::string> packages;
  std::map<std::string, vnvFullJsonStrCallback> jsonCallbacks;

  std::set<std::string> registeredPackages;
  int initializedCount = 0;
  bool runTests; /**< Should tests be run */
  bool terminalSupportsAsciiColors = true;
  int cleanupActionCounter = 0;
  json template_patch = json::object();

  UnitTestInfo unitTestInfo;
  RunTimeOptions runTimeOptions;
  RunInfo info;

  std::shared_ptr<VnVProv> prov;

  std::string configFile = "<unknown>";
  std::string hotpatchVar;

  // Workflow Identification
  std::string workflowName_;;
  std::string workflowJob_;
  std::string workflowDir_ = "/tmp";
  std::shared_ptr<JobManager> jobManager = nullptr;

  std::vector<std::string> command_line_vector;
  std::vector<char*> command_line_char_star;
  int command_line_size; 

  bool hotpatch;

  void loadRunInfo(RunInfo& info, registrationCallBack callback);

  void writeRunInfoFile();

  void makeLibraryRegistrationCallbacks(std::map<std::string, std::string> packageNames);
  bool configure(std::string packageName, RunInfo info, registrationCallBack callback);

 public:
  /**
   * @brief Init
   * @param argc The initialization arguements provided to the command line.
   * @param argv The command line args list
   * @param configFile The configuration file
   * @return true is initialization was successfull.
   *
   * Initialize the Runtime environment. This includes parsing and validating
   * the input file, loading the additional libraries, populating the test
   * store, selecting and configuring the IO Engine, etc.
   *
   * At the end of this function, a single stage INJECTION_POINT is included.
   * This injection point allows the user to add tests for writing input
   * information. The Provenance test included in the tests/provenance is
   * designed to work with this injection point in mind.
   */
  bool InitFromFile(const char* packageName, int* argc, char*** argv, std::string configFile,
                    InitDataCallback icallback, registrationCallBack callback);
  bool InitFromJson(const char* packageName, int* argc, char*** argv, json& configFile, InitDataCallback icallback,
                    registrationCallBack callback);

  int registerCleanUpAction(std::function<void(ICommunicator_ptr)> action);

  void declarePackageJson(std::string pname, vnvFullJsonStrCallback callback);

  void declareCommunicator(std::string pname, std::string commPack, std::string comm);

  bool useAsciiColors();

  long currentTime();

  std::string workflowName() { return workflowName_; }
  std::string workflowJob() { return workflowJob_; }

  /**
   * @brief printRunTimeInformation
   * Write all run infomation to the logs.
   */
  void printRunTimeInformation();

  void writeSpecification(std::string filename, bool quit);

  json getFullJson();

  json getFullJsonSchema();

  RunTimeOptions* getRunTimeOptions();

  void processToolConfig(json config, ICommunicator_ptr world);

  void runTimePackageRegistration(std::string packageName, registrationCallBack reg);

  bool isInitialized() { return initializedCount > 0; }

  /****************** ITERATIONS ************************************/
 private:
  std::shared_ptr<IterationPoint> getNewInjectionIteration(VnV_Comm comm, std::string pname, std::string id,
                                                           struct VnV_Function_Sig pretty, InjectionPointType type,
                                                           int once, NTV& args);

 public:
  int injectionIterationRun(VnV_Iterator* iterator);

  VnV_Iterator injectionIteration(VnV_Comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                                  std::string fname, int line, const DataCallback& callback, NTV& args, int once);

  /****************** PLUGS ************************************/
 private:
  std::shared_ptr<PlugPoint> getNewInjectionPlug(VnV_Comm comm, std::string pname, std::string id,
                                                 struct VnV_Function_Sig pretty, NTV& args);

 public:
  // Cpp Interface for an Injection Plug
  VnV_Iterator injectionPlug(VnV_Comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                             std::string fname, int line, const DataCallback& callback, NTV& args);

  int injectionPlugRun(VnV_Iterator* iterator);

  /************************ INJECTION POINTS **************************/
 private:
  std::shared_ptr<InjectionPoint> getNewInjectionPoint(VnV_Comm comm, std::string pname, std::string id,
                                                       struct VnV_Function_Sig pretty, InjectionPointType type,
                                                       NTV& args);

  std::shared_ptr<InjectionPoint> getExistingInjectionPoint(std::string pname, std::string id, InjectionPointType type,
                                                            std::string stageId);

  static RunTime& instance(bool reset);

 public:
  void injectionPoint(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                      std::string fname, int line, const DataCallback& callback, NTV& args);

  void injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id, struct VnV_Function_Sig pretty,
                            std::string fname, int line, const DataCallback& callback, NTV& args);

  void injectionPoint_end(std::string pname, std::string id, std::string fname, int line, const DataCallback& callback);

  void injectionPoint_iter(std::string pname, std::string id, std::string iterid, std::string fname, int line,
                           const DataCallback& callback);

  /**
   * @brief Finalize
   * @return
   *
   * Finalize the RunTime Environment. This includes finalizing the
   * Engine, and, if we initialized it, finalizing MPI.
   *
   */
  bool Finalize();

  /**
   * @brief isRunTests
   * @return True if tests should be run
   *
   * Should tests be run (as configured by the user input file).
   *
   */
  bool isRunTests();

  /**
   * @brief log
   * @param level
   * @param message
   * @param args
   *
   * Log a message with the logger. Note. Individual libraries can also use the
   * logger.
   *
   *
   */
  void log(VnV_Comm comm, std::string pname, std::string level, std::string message, va_list args);

  void log(VnV_Comm comm, std::string pname, std::string level, std::string message);

  void logUnhandled(std::string name, std::string id, NTV& args);

  void registerLogLevel(std::string packageName, std::string logLevel, std::string color);

  void registerFile(VnV_Comm comm, std::string packageName, std::string name, int input, std::string reader,
                    std::string infilename, std::string outfilename);

  std::map<std::type_index, std::unique_ptr<BaseStore>> stores;
  void resetStore();

  template <typename T> T* store_ptr() {
    auto it = stores.find(typeid(T));
    if (it != stores.end()) {
      return (T*)it->second.get();
    } else {
      stores[typeid(T)] = std::make_unique<T>();
      return (T*)stores.find(typeid(T))->second.get();
    }
  }

  template <typename T> T& store() { return *store_ptr<T>(); }

  /**
   * @brief instance
   * @return
   *
   * Get an instance of the RunTime class (only one exists).
   */
  static RunTime& instance();

  static RunTime& reset();


  bool loadPlugin(std::string filename, std::string packageName);

  /**
   * @brief runUnitTests
   *
   * Run all user configured unit testers.
   */
  void runUnitTests(VnV_Comm comm, UnitTestInfo info);

  void getFullSchema(std::string filename);

  void loadHotPatch(VnV_Comm comm);

  VnVProv getProv();

  std::string getPackageName();
};
}  // namespace VnV

#endif
