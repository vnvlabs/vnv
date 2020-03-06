
/**
  @file Runtime.h
**/


#ifndef VV_RUNTIME_HEADER
#define VV_RUNTIME_HEADER

/**
  \file VV-Runtime class.
  */
#include <map>
#include <stdarg.h>
#include <string>
#include <json-schema.hpp>
#include "base/JsonParser.h"
#include "base/Logger.h"
#include "base/InjectionPoint.h"
#include "c-interfaces/RunTime.h"
#include "c-interfaces/CJson.h"
#include "c-interfaces/CppInjection.h"
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
    void fromJson(json& jsonObject);
    static void callback(json &j);
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

  int* argc;    /**< Store the input parameters -- no idea why TODO*/
  char*** argv; /**< Stored args list for command line */

  bool runTests; /**< Should tests be run */
  bool finalize_mpi = false; /**< Are we responsible for calling MPI_Finalize) */
  bool logUnhandledInjectionPoints = true;
  bool terminalSupportsAsciiColors = true;


  void loadRunInfo(RunInfo &info, registrationCallBack *callback);
  void makeLibraryRegistrationCallbacks(std::map<std::string,std::string> packageNames);

  void getNewInjectionPoint(std::string pname, std::string id, InjectionPointType type, NTV &args);
  void getExistingInjectionPoint(std::string pname, std::string id, InjectionPointType type, std::string stageId);

  // TODO Getter and setter.
  RunTimeOptions runTimeOptions;

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
  bool Init(int* argc, char*** argv, std::string configFile, registrationCallBack *callback);

  bool useAsciiColors();
  /**
   * @brief printRunTimeInformation
   * Write all run infomation to the logs.
   */
  void printRunTimeInformation();

  void processToolConfig(json config);

  /**
   * @brief injectionPoint
   * @param injectionIndex The stage of the injection point to be run
   * @param id The name of the injection point to be run
   * @param argp The va_list containing all the arguements supplied at the
   * injection point.
   *
   * This function is called (eventually) whenever an INJECTION_POINT is found.
   * Here, we are responsibe for pulling the correct IP from the IPStore, and
   * running it with the given parameters.
   *
   */
  void injectionPoint(std::string pname, std::string id, NTV &args);
  void injectionPoint_begin(std::string pname, std::string id, NTV &args);
  void injectionPoint_end(std::string pname, std::string id);
  void injectionPoint_iter(std::string pname, std::string id, std::string iterid);

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
   * Log a message with the logger. Note. Individual libraries can also use the logger.
   *
   *
   */
  void log(std::string pname, std::string level, std::string message, va_list args);

  void logUnhandled(std::string name, std::string id, NTV &args);

  void registerLogLevel(std::string logLevel, std::string color);

  int beginStage(std::string pname, std::string message, va_list args);
  void endStage(int ref);

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
  void runUnitTests();
};
}  // namespace VnV


#endif
