#ifndef VV_LOGGING_HEADER
#define VV_LOGGING_HEADER


#  include <map>
#  include <string>
#  include <set>
#  include <iostream>

#  include "VnV-Interfaces.h"

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
   * @brief logLevelFromString
   * @param level String representation of the level requested
   * @return The requested log level.
   *
   * Utility function for converting a string into a LogLevel.
   *
   */
   LogLevel logLevelFromString(std::string level);

   /** @brief logLevelToString
   * @param level Utility functions for switching between strings and LogLevel
   * enum
   * @return LogLevel::X -> "X"
   *
   * Convert a LogLevel enum into a string.
   */
   std::string logLevelToString(LogLevel level);

   std::string getIndent(int stage);

   /**
 *  Internal. An internal namespace is used in a effort to "hide"
 * the Logger class. All logging should go through the Logging macros defined
 * above. (so they can be removed at compile time when needed)
 */

/**
 * @brief The LogLevel enum
 *
 * Enum Class defining the log levels available.
 */

/**
 * @brief The Logger Class
 *
 * The logger class does all the logging. The class itself consists of only
 *  functions; however, it should never be called directly. Instead, the
 * macros should be used to call the Logger.
 */
class Logger {
 private:
   friend class RunTime;

   Logger();
   bool engine = false; /**< True if this logger writes to the output engine. */
   int stage = 0;
   bool on = true; /**< Is the logger on */
   std::map<LogLevel, bool> logs; /**< Switches for the different log levels */
   std::ostream* fileptr; /**< ostream for writing the logs to the intended location */
   bool locked = false; /**< has the logger been configured */
   std::set<std::string> packageBlackList;
   std::string outFileName;
   /**




  /**
   * @arg level The level to which this log should be written.
   * @arg format. A std::string representing the fmt library format string.
   * @tparam Args Additional arguements to be passed to the fmt library.
   *
   * The Log function.
   */
   void log(std::string pname, LogLevel level, std::string format);

  /**
   * @brief setLog
   * @param outputType the type of output
   *
   * DO NOT CALL DIRECTLY, CALL THROUGH MACRO INSTEAD. Configure the log.
   * This is called from the JsonParser to configure
   * the logs based on the users input information. There should be
   * not need to call this anywhere else.
   */
   void setLog(const std::string& outputType);

  /**
   * @brief addToBlackList
   * @param packageName
   *
   * DO NOT CALL DIRECTLY. Adds a package to the blacklist for logging. Any library can use
   * the VnV Logger to log certain aspects of the code. This allows for consolidated logging
   * across multiple packages (although, only C++ is supported for now). vv-logging.h has
   * a variable called PACKAGE_NAME. That variable is set during compilation to be the name
   * of the project. Inside the configuration file, users can turn off logging from entire
   * packages by adding the package to the "blackList". For example, to turn off logging for
   * the internal VnV library, the user should add "VnV" to the blacklist. All debug statements
   * are written to file in the format [PACKAGE_NAME:LogLevel] <msg>, where PACKAGE_NAME is the name
   * of the package that should be blacklisted to turn of the logging.
   */
   void addToBlackList(std::string packageName);

  /**
   * @brief setLogLevel
   * @param level  what level should be modified
   * @param on should logging be turned on ?
   *
   * DO NOT CALL DIRECTLY. Set the flags for a given log level.
   */
   void setLogLevel(LogLevel level, bool on);

  /**
   * @brief log_c
   * @param level
   * @param format
   * @param args
   *
   * Standard C Style logging. Added for supporting C Programs. C++ programs should
   * use the C++ fmt library style.
   */
   void log_c(std::string pname, LogLevel level, std::string format, va_list args);


   /**
    * @brief print
    * Print out Logger configuration information.
    */
   void print();

};


};  // namespace VnV/**

#endif
