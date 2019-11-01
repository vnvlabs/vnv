#ifndef VV_LOGGING_HEADER
#define VV_LOGGING_HEADER

#ifdef WITH_LOGGING

#  include <fmt/format.h>

#  include <map>
#  include <string>

/**
 * \file Header file for the logging structure in the code. The logger itself is
 * a c++ class, however, all calls to the logger should be completed through
 * the macros defined in this class. Using macros to access the logger is ideal
 * because a simple compiler flag can be used to remove all logging calls from
 * the compiled library.
 */

/**
 * VnV_Configure This define is called once during configuration from inside
 * the VnV::JsonParser. This sets the type of log. Supported types are "stdout",
 * "stderr" and "<filename>" where filename is the desired log file name. If
 * cofiguration is not completed, the logger is not initialized and no logging
 * takes place.
 */
#  define VnV_ConfigureLog(type) VnV::Internal::Logger::setLog(type);

/**
 * VnV_ConfigureLogLevel This Macros allows the user to turn logging on and off
   for each level. Here level is a string "DEBUG","WARN","INFO","ERROR". on is a
 boolean that either turns on (true) or off (false) logging for that level.
 */
#  define VnV_ConfigureLogLevel(level, on) \
    VnV::Internal::Logger::setLogLevel(level, on);

/**
 * Write a log to the debug log. See Logger::log for information
 * on formating and writing log files.
 */
#  define VnV_Debug(...) VnV::Internal::Logger::debug(__VA_ARGS__);
/**
  *  Write a log to the warn log.See Logger::log for information
  * on formating and writing log files.

  */
#  define VnV_Warn(...) VnV::Internal::Logger::warn(__VA_ARGS__);
/**
  *  Write a log to the info log.See Logger::log for information
  * on formating and writing log files.

  */
#  define VnV_Info(...) VnV::Internal::Logger::info(__VA_ARGS__);
/**
 *  Write a log to the error log.See Logger::log for information
 * on formating and writing log files.
 */
#  define VnV_Error(...) VnV::Internal::Logger::error(__VA_ARGS__);

/**
 * VnV Namespace.
 */
namespace VnV {

/**
 *  Internal. An internal namespace is used in a effort to "hide"
 * the Logger class. All logging should go through the Logging macros defined
 * above. (so they can be removed at compile time when needed)
 */
namespace Internal {

/**
 * @brief The LogLevel enum
 *
 * Enum Class defining the log levels available.
 */
enum class LogLevel { DEBUG, INFO, WARN, ERROR };

/**
 * @brief The Logger Class
 *
 * The logger class does all the logging. The class itself consists of only
 * static functions; however, it should never be called directly. Instead, the
 * macros should be used to call the Logger.
 */
class Logger {
 private:
  static bool on; /**< Is the logger on */
  static std::map<LogLevel, bool>
      logs; /**< Switches for the different log levels */
  static std::ostream*
      fileptr; /**< ostream for writing the logs to the intended location */
  static bool locked; /**< has the logger been configured */

  /**
   * @brief logLevelToString
   * @param level Utility functions for switching between strings and LogLevel
   * enum
   * @return LogLevel::X -> "X"
   *
   * Convert a LogLevel enum into a string.
   */
  static std::string logLevelToString(LogLevel level);

  /**
   * @brief logLevelFromString
   * @param level String representation of the level requested
   * @return The requested log level.
   *
   * Utility function for converting a string into a LogLevel.
   *
   */
  static LogLevel logLevelFromString(std::string level);

  /**
   * @arg level The level to which this log should be written.
   * @arg format. A std::string representing the fmt library format string.
   * @tparam Args Additional arguements to be passed to the fmt library.
   *
   * The Log function. Ultimately, it is this function that writes the requested
   * information to the configured ostream. Simple logs can be created by simply
   * supplying a string.
   *
   * However, the logs support the full range of formating options provided by
   * the fmt library. See fmt-library for more details. A simple example would
   * be log(LogLevel::DEBUG, "hello {}, "world") -> hello world.
   */
  template <typename... Args>
  static void log(LogLevel level, std::string format, Args&&... args) {
    if (!locked) return;

    auto it = logs.find(level);
    if (it != logs.end() && !it->second) return;

    std::string outstring = fmt::format(format, std::forward<Args>(args)...);
    (*fileptr) << "[" << logLevelToString(level) << "] " << outstring
               << std::endl;
  }

 public:
  /**
   * @brief setLog
   * @param outputType the type of output
   *
   * DO NOT CALL DIRECTLY, CALL THROUGH MACRO INSTEAD. Configure the log.
   * This is called from the JsonParser to configure
   * the logs based on the users input information. There should be
   * not need to call this anywhere else.
   */
  static void setLog(const std::string& outputType);

  /**
   * @brief setLogLevel
   * @param level  what level should be modified
   * @param on should logging be turned on ?
   *
   * DO NOT CALL DIRECTLY. Set the flags for a given log level.
   */
  static void setLogLevel(const std::string& level, bool on);

  /**
   * Conveinence function for writing to debug logs
   */
  template <typename... Args>
  static void debug(std::string format, Args&&... args) {
    log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
  }

  /**
   * Conveinence function for writing to warn logs
   */
  template <typename... Args>
  static void warn(std::string format, Args&&... args) {
    log(LogLevel::WARN, format, std::forward<Args>(args)...);
  }

  /**
   * Conveinence function for writing to info logs
   */
  template <typename... Args>
  static void info(std::string format, Args&&... args) {
    log(LogLevel::INFO, format, std::forward<Args>(args)...);
  }

  /**
   * Conveinence function for writing to error logs
   */
  template <typename... Args>
  static void error(std::string format, Args&&... args) {
    log(LogLevel::ERROR, format, std::forward<Args>(args)...);
  }
};

};  // namespace Internal

};  // namespace VnV

#else
/**
 * Define all logging command to nothing. This removes logging from the build
 * entirely. Note: The build will not compile if the logging class was used
 * directly at any point.
 */
#  define VnV_ConfigureLog(type)
#  define VnV_ConfigureLogLevel(level, on)
#  define VnV_Debug(...)
#  define VnV_Warn(...)
#  define VnV_Info(...)
#  define VnV_Error(...)

#endif

#endif
