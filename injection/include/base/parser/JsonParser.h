/**
  @file JsonParser.h
**/

#ifndef VV_PARSER_HEADER
#define VV_PARSER_HEADER

/**
  \file vv-parser.h Header file for the Parser.
  */

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

struct LoggerInfo {
  bool on;
  std::string filename;
  std::map<std::string, bool> logs;
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

struct SamplerInfo {
  std::string name = "";
  std::string package = "";
  nlohmann::json config = json::object();
};

enum class InjectionType { POINT, ITER, PLUG };

struct InjectionPointInfo {
  InjectionType type = InjectionType::POINT;
  std::string name;
  std::string package;
  std::vector<json> tests;
  std::vector<json> iterators;
  json plug = json::object();

  SamplerInfo sampler;

  bool runInternal;
};

struct UnitTestInfo {
  bool runUnitTests;
  json unitTestConfig;
  bool exitAfterTests;
};

struct ActionConfig {
  json config;
  std::string name;
  std::string package;
  std::string run = "";
};

struct ActionInfo {
  bool run;
  std::vector<ActionConfig> actions;
};

/**
 * @brief The EngineInfo struct
 *
 * Utility struct for storing parsed information about the RunTime Env
 */
struct RunInfo {
  bool runTests; /**< Should any tests be run */

  std::string communicator =
      "mpi";  //**< should we use mpi. If yes, vnv will use mpi communicator. if
              // false, we will use serial.

  std::map<std::string, std::string>
      additionalPlugins; /*< List of file paths to included plugin libraries */
  std::map<std::string, InjectionPointInfo>
      injectionPoints; /**< all injection points with tests */

  json pluginConfig;
  json cmdline;

  UnitTestInfo unitTestInfo;
  LoggerInfo logInfo;
  EngineInfo engineInfo; /**< Information about the IO engine */

  ActionInfo actionInfo;

  json template_overrides = json::object();

  bool hotpatch = false;

  bool error; /**< Was there an error when parsing */
  std::string
      errorMessage; /**< What was the error message (if there was one) */
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

  ActionInfo getActionInfo(const json& actionJson, std::string type);

  SamplerInfo getSamplerInfo(const json& samplerJson);

  /**
   * @brief getUnitTestInfo
   * @param unitTestJson The json found in main["unit-testing"]
   * @return
   */
  UnitTestInfo getUnitTestInfo(const json& unitTestJson);

  /**
   * @brief addTest
   * @param testJson[in] The json extracted from the tests section of the
   * injection point spec.
   * @param testConfig[out]
   * @param runScopes[in] A list of runscopes specified for execution in this
   * run
   *
   * Here, we parse the configuation for a single test and add it to the
   * testConfig vector. Note: If the runScopes vector is not empty, the test
   * will only be added if it has been tagged with a runScope in that set. If
   * runScopes is empty, the test is added regardless.
   */
  void addTest(const json& testJson, std::vector<json>& testConfig,
               std::set<std::string>& runScopes);

  bool add(const json& testJson, std::set<std::string>& runScopes);

  /**
   * @brief addInjectionPoint
   * @param injectionPointJson[in] The json extracted for this injection point
   * spec.
   * @param runScopes[in] The active runScopes.
   * @param ips[in/out] A map that the injection point should be added to.
   *
   * Here we add the injection point to the injection point list. If no tests
   * are defined for the injection point, then the injection point is not added.
   * At runtime, the VnV library will simply move on when an unconfigured
   * injection point is found.
   */
  void addInjectionPoint(const json& injectionPointJson,
                         std::set<std::string>& runScopes,
                         std::map<std::string, InjectionPointInfo>& ips,
                         InjectionType type);

  /**
   * @brief addTestLibrary
   * @param testLibsJson the json extracted from the input file.
   * @param libs The vector of libraries to add the new test library to
   *
   * Here, add the newest filepath to the testLibraries set.
   */
  void addTestLibrary(const json& testLibsJson,
                      std::map<std::string, std::string>& libs);

  /**
   * @brief parse
   * @param input The json provided by the user
   * @return The RunInfo Struct
   *
   * Parse the user specificed json file. As part of parsing, the json is
   * validated against the vv-schema json-schema to ensure it is accurate. Note
   * that in some cases, the functions in this class assume a variable is
   * present in the json. In most cases, those assumptions are based on the fact
   * that the parameter is marked required in the schema.
   */
  RunInfo _parse(const json& input, int* argc, char** argv);

 public:
  /**
   * @brief parse
   * @param fstram . The file containing the json or a json string.
   * @return The RunInfo struct containing all parsed information.
   *
   * Note, This function supports both files and json strings as input.
   * @todo Change parameter name to indicate support for json strings and files.
   */

  json commandLineParser(int* argc, char** argv);
  RunInfo parse(std::ifstream& fstream, int* argc, char** argv);
  RunInfo parse(const json& _json, int* argc, char** argv);
};

}  // namespace VnV

#endif
