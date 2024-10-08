﻿
/**
  @file Logger.h
**/

#ifndef VV_LOGGING_HEADER
#define VV_LOGGING_HEADER

#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>

#include "base/parser/JsonParser.h"
#include "common-interfaces/all.h"
#include "base/communication/ICommunicator.h"
#define MAXSAVED_LOGS 1024

#define MAX_LOG_SIZE 2048

/**
 * \file Header file for the logging structure in the code. The logger itself is
 * a c++ class, however, all calls to the logger should be completed through
 * the macros defined in this class. Using macros to access the logger is ideal
 * because a simple compiler flag can be used to remove all logging calls from
 * the compiled library.
 */

/**
 * VnV Namespace.
 */
namespace VnV {

/**
 *  Internal. An internal namespace is used in a effort to "hide"
 * the Logger class. All logging should go through the Logging macros defined
 * above. (so they can be removed at compile time when needed)
 */

/**
 * @brief The Logger Class
 *
 * The logger class does all the logging. The class itself consists of only
 *  functions; however, it should never be called directly. Instead, the
 * macros should be used to call the Logger.
 *
 *
 *
 */
class Logger {
 private:
  friend class RunTime;
  friend class IUnitTester;  // Allow unit tester to Test the private functions.
  int refcount = 0;
  int rank = -1;

  std::map<std::string, std::string> logLevelsToColor;
  std::map<std::string, bool> logs; /**< Switches for the different log levels */
  std::queue<std::tuple<std::string, int, std::string, std::string, VnV_Comm>>
      savedLogs; /** logging statements that occur before engine config. **/
  Logger();

  bool engine = false; /**< True if this logger writes to the output engine. */
  LogWriteType ltype;

  int stage = 0;

  bool on = true;        /**< Is the logger on */
  std::ostream* fileptr; /**< ostream for writing the logs to the intended location */
  bool locked = false;   /**< has the logger been configured */
  std::set<std::string> packageBlackList;
  std::string outFileName;

  void setRank(int r) { rank = r; }

  /**
   * @arg level The level to which this log should be written.
   * @arg format. A std::string representing the fmt library format string.
   * @tparam Args Additional arguements to be passed to the fmt library.
   *
   * The Log function.
   */
  void log(VnV_Comm comm, std::string pname, std::string level, std::string format, bool saved=true);

  /**
   * @brief setLog
   * @param outputType the type of output
   *
   * DO NOT CALL DIRECTLY, CALL THROUGH MACRO INSTEAD. Configure the log.
   * This is called from the JsonParser to configure
   * the logs based on the users input information. There should be
   * not need to call this anywhere else.
   */

  void setLog(bool engine, LogWriteType t, const std::string& outputType = "");

  void write_to_file(ICommunicator_ptr comm, std::string pname, int stage, std::string level, std::string message);
  
  /**
   * @brief addToBlackList
   * @param packageName
   *
   * DO NOT CALL DIRECTLY. Adds a package to the blacklist for logging. Any
   * library can use the VnV Logger to log certain aspects of the code. This
   * allows for consolidated logging across multiple packages (although, only
   * C++ is supported for now). vv-logging.h has a variable called PACKAGE_NAME.
   * That variable is set during compilation to be the name of the project.
   * Inside the configuration file, users can turn off logging from entire
   * packages by adding the package to the "blackList". For example, to turn off
   * logging for the internal VnV library, the user should add "VnV" to the
   * blacklist. All debug statements are written to file in the format
   * [PACKAGE_NAME:logLevel] <msg>, where PACKAGE_NAME is the name of the
   * package that should be blacklisted to turn of the logging.
   */
  void addToBlackList(std::string packageName);

  /**
   * @brief log_c
   * @param level
   * @param format
   * @param args
   *
   * Standard C Style logging.
   */
  void log_c(VnV_Comm comm, std::string pname, std::string level, std::string format, va_list args);

  /**
   * @brief print
   * Print out Logger configuration information.
   */
  void print();

  void registerLogLevel(std::string packageName, std::string name, std::string color, bool on);
  void setLogLevel(std::string level, bool on);
  std::string logLevelToColor(std::string logLevel, std::string message);

  std::string getIndent(int stage);

  void up();
  void down();
};

}  // namespace VnV

#endif
