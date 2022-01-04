/**
  @file JsonParser.h

  Structures and Classes required to parse the VnV Json Input file.

**/

#ifndef VV_PARSER_HEADER
#define VV_PARSER_HEADER

#include <fstream>
#include <map>
#include <set>
#include <string>

#include "json-schema.hpp"
using nlohmann::json;

/**
 * VnV namespace
 */
namespace VnV {

/**
 * @brief Logging information
 *
 * Store user provided logging configuration
 *
 */

enum class LogWriteType {
  STDOUT,  //< All logging statements are written to stdout
  FILE,    //< All logging statements are written to file
  ENGINE   //< All logging statements are forwarded to output engine.
};
struct LoggerInfo {
  /**
   * @brief If true, logging is enabled.
   *
   * When false, all VnV logging is disabled. All calls to VnV_Log
   * become noops.
   *
   */
  bool on;

  /**
   * @brief Where should we write logging statements.
   */
  LogWriteType type = LogWriteType::STDOUT;

  /**
   * @brief Log filename
   *
   * The filename used to write log files when logging is enabled in filename
   * @note Only used when VnV::LoggerInfo::type == VnV::LogWriteType::FILE
   */
  std::string filename; /**< The filename to write log files to (if applicable) */

  /**
   * @brief Package specific log configuration
   *
   * A map linking log level to logging status.
   * If logs[LOG LEVEL] is false, all logging statements emitted with that
   * log level will be ignored.
   *
   */
  std::map<std::string, bool> logs;

  /**
   * @brief Logging Package black list
   *
   * Logging statments emitted by packages in the blacklist will be ignored.
   * This allows users to turn logging off logging for certain packages.
   */
  std::set<std::string> blackList;
};

/**
 * @brief The EngineInfo struct
 *
 * Utility struct for storing parsed information about the Engine.
 */
struct EngineInfo {
  std::string engineType; /**< The type of IO engine to use */
  json engineConfig;      /**< additional parameters provided by the user */
};

/**
 * @brief Structure containing configuration for a VnV Sampler.
 *
 * Configuration options are a VnV::ISampler.
 * \sa VnV::ISampler
 *
 */
struct SamplerInfo {
  std::string name = "";                  /**< The name of the sampler*/
  std::string package = "";               /**< the package that defined the sampler */
  nlohmann::json config = json::object(); /** sampler specific configuration options*/
};

/**
 * @brief The type of injection point defined.
 *
 */
enum class InjectionType {
  POINT, /**< A Simple Injection Point (see VnV::InjectionPoint)*/
  ITER,  /**< A VnV Iteration Point (see VnV::IterationPoint)*/
  PLUG   /**< A VnV Plug point (see VnV::PlugPoint) */
};

/**
 * @brief Injection Point configuration
 *
 * The configuration information for a injection point
 *
 */
struct InjectionPointInfo {
  InjectionType type = InjectionType::POINT; /**< injection point type */
  std::string name;                          /**< The name of the injection point */
  std::string package;                       /**< The package that declared the injection point */
  std::vector<json> tests;                   /**< Test configuration json defined at the injection point */
  std::vector<json> iterators;               /**< Iteration configuration json defined at the injection point */
  json plug = json::object();                /**< Plug configuration json defined at the injection point */
  json templateName = json::object();
  SamplerInfo sampler; /**< Sampler configuration for this injection point */

  bool runInternal; /** Should we run the internal callback for this injection point */
};

/**
 * @brief Unit Testing Configuration
 *
 * The configuration for any unit testing that needs to be completed
 * during this execution of the code base.
 *
 */
struct UnitTestInfo {
  bool runUnitTests;   /**< if true, unit tests will be executed */
  json unitTestConfig; /**< the configuration object for the unit tests */
  bool exitAfterTests; /**< if true, std::exit will be called at end of tests */
};

/**
 * @brief A configuration object for an action
 *
 * Actions are plugin provided functions that can be executed at various
 * points inside the code base. Actions are almost identical to test. In fact,
 * one usage of an action might be to perform a global test across all injection
 * points.
 *
 */
struct ActionConfig {
  json config;         /**< the action specific configuration object */
  std::string name;    /**< the name of the action to run */
  std::string package; /**< the package that declared the action */
  bool run = false;    /**< when should the action be executed (todo --> enum )*/
};

/**
 * @brief Global Action Configuration
 *
 */
struct ActionInfo {
  bool run;                          /**< if true, actions will be executaed*/
  std::vector<ActionConfig> actions; /**< the list of action configuration to execute */
};

/**
 * @brief Runtime configuration options
 *
 * Utility struct for storing parsed information about the RunTime Environment
 */
struct RunInfo {
  bool runTests; /**< Should any tests be run */

  std::string communicator = "mpi";                          /**< what communicator should be used*/
  std::map<std::string, std::string> additionalPlugins;      /**< List of file paths to included plugin libraries */
  std::map<std::string, InjectionPointInfo> injectionPoints; /**< all injection points with tests */

  json pluginConfig; /**< Json object mapping packageName to shared library path */
  json cmdline;      /**< Command Line options parsed into a configuration json --vnv.packageName.[sdfsdf] = "sdf"*/

  UnitTestInfo unitTestInfo; /**< Unit testing configuration */
  LoggerInfo logInfo;        /**< loging configuration */
  EngineInfo engineInfo;     /**< Information about the IO engine */

  ActionInfo actionInfo; /**< action configuration */

  /**
   * @brief User provided template overrides
   *
   * We merge the template overrides into the template json extracted from the application.
   * This allows the user to change the templates that are used for a test, injection point, etc.
   *
   */
  json template_overrides = json::object();

  bool hotpatch = false; /**< Should hotpatching be enabled */

  bool error;               /**< Was there an error when parsing */
  std::string errorMessage; /**< What was the error message (if there was one) */
};

/**
 * @brief The JsonParser class
 *
 * The JsonParser class handles Parsing of the users input file. The parser
 * relies on two third party libraries -- nlohmann-json and
 * json-schema-validator. The first is a general purpose json parser. The second
 * is a library for validating json objects against a json_schema document
 * (similar to xsd for xml).
 *
 * The schema used to validate the VnV input file is provided in vv-schema.h.
 * Additional validation is completed by some sub classes (see vv-output.h) for
 * an exmaple.
 *
 * This parser supports an option called runScopes. This is a parser level
 * construct that does not propergate into the overall library. The runscopes
 * parameter allows the user to turn on/off tests in the configuration file
 * based on a key word.
 *
 * All tests can be marked with a runScope and the user can supply a list of
 * scopes to run. A test is only added for execution if its runscope is supplied
 * in the users list of runscopes. An empty runscopes list means all scopes are
 * run. To turn of tests altogether set the runTests boolean in the json file.
 */
class JsonParser {
 private:
  /**
   * @brief setupLogger
   * @param loggingJson The Json pulled from the main["logging"] section of the
   * input file.
   *
   * This function parses the logging configuration information provided by the
   * user in the input file and passes it to the Logger. This is in contrast to
   * the other functions in this class, which simply store the parsed
   * information into the RunInfo struct, while leaving the actual configuration
   * to another class (RunInfo::parse).
   *
   * The reason for this is that we want the logger to be configured as early as
   * possible. In particular, we want to be able to log the parser itself. As
   * such, the logging information is the first thing extracted from the input
   * file.
   *
   */
  LoggerInfo getLoggerInfo(const json& loggingJson);

  /**
   * @brief getEngineInfo
   * @param engineJson The Json extracted from main["engine"]
   * @return A EngineInfo struct describing the users desired Engine type and
   * parameters.
   *
   * Load the information about the engine.
   */
  EngineInfo getEngineInfo(const json& engineJson);

  /**
   * @brief Parse the Action section of the input file.
   * @param actionJson The Json extracted from main["actions"]
   * @param type The type of action being parsed (todo)
   * @return The actionJson input parsed into the ActionInfo object.
   *
   * Load the information about the engine.
   */

  ActionInfo getActionInfo(const json& actionJson);

  /**
   * @brief Get the Sampler Info object
   *
   *
   *
   * @param samplerJson User provided sampler configuration
   * @return SamplerInfo
   */
  SamplerInfo getSamplerInfo(const json& samplerJson);

  /**
   * @brief Get the Unit Test Info object
   *
   * @param unitTestJson User provided json object for unit test configuration
   * @return UnitTestInfo
   */
  UnitTestInfo getUnitTestInfo(const json& unitTestJson);

  /**
   * @brief add a test to an injection point
   *
   * Here, we parse the configuation for a single test and add it to the
   * testConfig vector. Note: If the runScopes vector is not empty, the test
   * will only be added if it has been tagged with a runScope in that set. If
   * runScopes is empty, the test is added regardless.
   *
   * @param[in] testJson  The json extracted from the tests section of the injection point spec.
   * @param[in] testConfig The list of current tests to add the test too.
   * @param[in] runScopes A list of runscopes specified for execution in this run
   *
   */
  void addTest(const std::string key, const json& testJson, std::vector<json>& testConfig,
               std::set<std::string>& runScopes);

  /**
   * @brief Determine if a test should be added based on the user supplied runscopes
   *
   * @param testJson The test configuration
   * @param runScopes The list of active runscopes
   * @return true The test should be added
   * @return false The test should not be added
   */
  bool add(const json& testJson, std::set<std::string>& runScopes);

  /**
   * @brief Add an injection point to the list of active injection points.
   *
   * @param[in] injectionPointJson The json extracted for this injection point spec.
   * @param[in] runScopes The active runScopes.
   * @param ips[in/out] The map that the injection point should be added to.
   *
   * Here we add the injection point to the injection point list.
   */
  void addInjectionPoint(const json& injectionPointJson, std::set<std::string>& runScopes,
                         std::map<std::string, InjectionPointInfo>& ips, InjectionType type);

  /**
   * @brief add a plugin
   * @param[in] testLibsJson the json extracted from the input file plugins section
   * @param[inout] libs The map of library name to shared library file.
   *
   * Here, add the newest filepath to the testLibraries set.
   *
   * @todo Change the name of this function to add Plugin.
   */
  void addTestLibrary(const json& testLibsJson, std::map<std::string, std::string>& libs);

  /**
   * @brief Parse the users input file.
   * @param[in] input The json provided by the user
   * @param[in] argc The number of command line arguments
   * @param[in] argv The command line arguments
   * @return The RunInfo Struct representing the users input file.
   *
   * Parse the user specificed json file. As part of parsing, the json is
   * validated against the vv-schema json-schema to ensure it is accurate.
   *
   * @todo argc should be an int, not an int*
   */
  RunInfo _parse(const json& input, int* argc, char** argv);

 public:
  /**
   * @brief Parse the command line
   *
   * Search through the command line for parameters of the form
   * --vnv.package.*=<value> and parse them into a Json object of the form
   * {"package"-> {key:value}}
   *
   * The command line json, along with any package options provided to the
   * user are sent through to the packages options callback.
   *
   * @param argc the number of arguments in argv
   * @param argv the command line arguments
   * @return json representation of the command line arguments
   *
   * @todo Remove this functionality -- Users can now pass override arguments
   * on the command line that are automatically merged into the input file. No
   * need to additionally support command line parameters with that functionality.
   */
  json commandLineParser(int* argc, char** argv);

  /**
   * @brief Parse a users input file given an input file stream
   *
   * @param fstream The valid input file stream
   * @param argc  The number of command line arguements
   * @param argv  The command line arguments
   * @return RunInfo
   */
  RunInfo parse(std::ifstream& fstream, int* argc, char** argv);

  /**
   * @brief Parse a users input file using a json object.
   *
   * @param fstream The valid input file stream
   * @param argc  The number of command line arguements
   * @param argv  The command line arguments
   * @return RunInfo
   */
  RunInfo parse(const json& _json, int* argc, char** argv);
};

}  // namespace VnV

#endif
