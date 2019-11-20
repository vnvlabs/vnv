
#ifdef WITH_LOGGING

/** @file vv-logging.cpp **/

#  include "vv-logging.h"

#  include <fstream>

using namespace VnV::Internal;

std::string Logger::logLevelToString(LogLevel level) {
  switch (level) {
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARN:
    return "WARN";
  case LogLevel::ERROR:
    return "ERROR";
  }
  throw "enum added to LogLevel without implementing to string method [see vv-logging.cpp]";
};

LogLevel Logger::logLevelFromString(std::string level) {
  if (level.compare("DEBUG") == 0) return LogLevel::DEBUG;
  if (level.compare("INFO") == 0) return LogLevel::INFO;
  if (level.compare("WARN") == 0) return LogLevel::WARN;
  if (level.compare("ERROR") == 0) return LogLevel::ERROR;
  throw "Invalid";
}

bool Logger::locked = false;
std::ostream* Logger::fileptr = nullptr;
std::map<LogLevel, bool> Logger::logs;

void Logger::setLogLevel(const std::string& level, bool on) {
  logs.insert(std::make_pair(logLevelFromString(level), on));
}

void Logger::setLog(const std::string& filename) {
  if (locked) {
    if (fileptr != nullptr && fileptr != &std::cout && fileptr != &std::cerr) {
      dynamic_cast<std::ofstream*>(fileptr)->close();
    }
  }
  locked = true;
  if (filename.compare("stdout") == 0) {
    fileptr = &std::cout;
  } else if (filename.compare("stderr") == 0) {
    fileptr = &std::cerr;
  } else {
    std::ofstream fp;
    fp.open(filename, std::ofstream::out | std::ofstream::app);
    fileptr = &fp;
  }
}

#endif
