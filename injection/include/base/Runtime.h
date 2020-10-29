
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

#include "base/InjectionPoint.h"
#include "base/JsonParser.h"
#include "base/Logger.h"
#include "c-interfaces/CJson.h"
#include "c-interfaces/RunTime.h"
#include "c-interfaces/Wrappers.h"
#include "interfaces/CppInjection.h"
#include "interfaces/IOffloader.hpp"
/**
 * VnV Namespace
 */

using nlohmann::json;

namespace VnV {

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

class RunTimeOptions {
 public:
  bool logUnhandled = false;
  std::string dumpConfigFilename = "";
  bool dumpConfig = false;
  void fromJson(json& jsonObject);
  static void callback(json& j);
};

class RunTime {
  friend class RunTimeOptions;

 private:
  Logger logger;

  /**
   * @brief RunTime
   *
   * Private Constructor -- Called used the getRunTime() function.
   *
   */
  RunTime();

  std::string mainPackageName;
  std::set<std::string> plugins;
  std::set<std::string> packages;
  std::map<std::string, vnvFullJsonStrCallback> jsonCallbacks;

  std::shared_ptr<IOffloader> offloader;

  std::set<std::string> registeredPackages;

  bool runTests; /**< Should tests be run */
  bool terminalSupportsAsciiColors = true;

  void loadPlugin(std::string filename, std::string packageName);

  void loadRunInfo(RunInfo& info, registrationCallBack* callback);
  void makeLibraryRegistrationCallbacks(
      std::map<std::string, std::string> packageNames);

  std::shared_ptr<InjectionPoint> getNewInjectionPoint(std::string pname,
                                                       std::string id,
                                                       InjectionPointType type,
                                                       NTV& args);
  std::shared_ptr<InjectionPoint> getExistingInjectionPoint(
      std::string pname, std::string id, InjectionPointType type,
      std::string stageId);

  // TODO Getter and setter.
  RunTimeOptions runTimeOptions;

  bool configure(std::string packageName, RunInfo info,
                 registrationCallBack* callback);

  bool setupOffloadConfiguration(OffloadInfo& info);

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
  bool InitFromFile(const char* packageName, int* argc, char*** argv,
                    std::string configFile, registrationCallBack* callback);
  bool InitFromJson(const char* packageName, int* argc, char*** argv,
                    json& configFile, registrationCallBack* callback);

  void declarePackageJson(std::string pname, vnvFullJsonStrCallback callback);

  void declareCommunicator(std::string pname, std::string commPack,
                           std::string comm);

  bool useAsciiColors();
  /**
   * @brief printRunTimeInformation
   * Write all run infomation to the logs.
   */
  void printRunTimeInformation();

  void writeSpecification(std::string filename);

  json getFullJson();

  RunTimeOptions* getRunTimeOptions();

  void processToolConfig(json config, json& cmdline);

  void runTimePackageRegistration(std::string packageName,
                                  vnv_registration_function reg);

  // Cpp interface.
  void injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                      const CppInjection::DataCallback& callback, NTV& args);
  void injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id,
                            const CppInjection::DataCallback& callback,
                            NTV& args);

  // C Interface
  void injectionPoint(VnV_Comm comm, std::string pname, std::string id,
                      injectionDataCallback* callback, NTV& args);
  void injectionPoint_begin(VnV_Comm comm, std::string pname, std::string id,
                            injectionDataCallback* callback, NTV& args);

  void injectionPoint_end(std::string pname, std::string id);
  void injectionPoint_iter(std::string pname, std::string id,
                           std::string iterid);

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
  void log(VnV_Comm comm, std::string pname, std::string level,
           std::string message, va_list args);

  void logUnhandled(std::string name, std::string id, NTV& args);

  void registerLogLevel(std::string packageName, std::string logLevel,
                        std::string color);

  int beginStage(VnV_Comm comm, std::string pname, std::string message,
                 va_list args);
  void endStage(VnV_Comm comm, int ref);

  /**
   * @brief instance
   * @return
   *
   * Get an instance of the RunTime class (only one exists).
   */
  static RunTime& instance();

  /**
   * @brief loadInjectionPoints
   * @param json
   *
   * Load injection points from a json file. This is a pooly named WIP.
   *
   * The idea here is that we can load additional injection points at any time.
   * At the moment this is used by the unit testers to add injection point
   * configuraitons dynamically.
   *
   *
   */
  void loadInjectionPoints(json _json);

  /**
   * @brief runUnitTests
   *
   * Run all user configured unit testers.
   */
  void runUnitTests(VnV_Comm comm);

  void readFile(std::string filename, long& idCounter);
};
}  // namespace VnV

#endif
