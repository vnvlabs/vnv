﻿
/** @file Logger.cpp Implementation of the Logger class as defined in
    base/Logger.h
**/
#include "base/Logger.h"  //Prototype.

#include <sstream>  //stringstream

#include "base/Runtime.h"                   // RunTime.
#include "base/stores/OutputEngineStore.h"  // OutputEngineStore
#include "c-interfaces/Logging.h"           // Logging macros
using namespace VnV;

namespace {
constexpr const char* red = "\033[31m";
constexpr const char* green = "\033[32m";
constexpr const char* blue = "\033[34m";
constexpr const char* magenta = "\033[35m";
constexpr const char* yellow = "\033[33m";
constexpr const char* white = "\033[37m";
constexpr const char* cyan = "\033[36m";
constexpr const char* reset = "\033[0m";
}  // namespace

std::string VnV::Logger::logLevelToColor(std::string level,
                                         std::string message) {
  
  if (!RunTime::instance().useAsciiColors()) return message;

  
  auto it = logLevelsToColor.find(level);
  if (it != logLevelsToColor.end()) {
    std::ostringstream oss;
    oss << it->second << message << reset;
    return oss.str();
  }
   return message;
};

void Logger::up() {
  stage++;
}
void Logger::down() {
  stage--;
}

Logger::Logger() {
  registerLogLevel(VNVPACKAGENAME_S, "INFO", green);
  registerLogLevel(VNVPACKAGENAME_S, "DEBUG", green);
  registerLogLevel(VNVPACKAGENAME_S, "WARN", blue);
  registerLogLevel(VNVPACKAGENAME_S, "ERROR", red);
};

void Logger::registerLogLevel(std::string packageName, std::string name,
                              std::string color) {
  logLevelsToColor[packageName + ":" + name] = color;
}

std::string Logger::getIndent(int stages) {
  std::string s = "";
  for (int i = 0; i < std::max(0, stages); i++) s += "----";
  return s;
}

void Logger::setLogLevel(std::string level, bool on) {
  logs.insert(std::make_pair(level, on));
}

void Logger::log(VnV_Comm comm, std::string pname, std::string level,
                 std::string format) {
  if (!locked) return;
  if (packageBlackList.find(pname) != packageBlackList.end()) {
    return;
  }
  auto it = logs.find(pname + ":" + level);
  if (it != logs.end() && !it->second) return;

  if (engine) {
    try {

      OutputEngineManager* eng =
          OutputEngineStore::instance().getEngineManager();

      // Clear any saved logs.
      while (savedLogs.size() > 0) {
        auto& t = savedLogs.front();
        VnV_Comm cc = std::get<4>(t);
        auto c = CommunicationStore::instance().getCommunicator(cc);
        eng->Log(c, std::get<0>(t).c_str(), std::get<1>(t), std::get<2>(t),
                 std::get<3>(t));
        savedLogs.pop();
      }

      auto c = CommunicationStore::instance().getCommunicator(comm);
      eng->Log(c, pname.c_str(), stage, level, format);
    } catch (...) {
      // Logging statements that occur prior to the engine being configured at
      // written to std::out.
      if (savedLogs.size() > MAXSAVED_LOGS) {
        std::cout << "Too Many Logs before engine configuration: Dumping to "
                     "stdout instead"
                  << std::endl;
        auto& t = savedLogs.front();
        std::ostringstream oss;
        std::string pkey = std::get<0>(t) + ":" + std::get<2>(t);
        oss << getIndent(std::get<1>(t)) << "[" << pkey << "](Rank: " << rank << ") " << std::get<3>(t);

        std::cout << logLevelToColor(pkey, oss.str()) << std::endl;
        savedLogs.pop();
      }
      savedLogs.push(std::make_tuple(pname, stage, level, format, comm));
    }
  } else {
    std::ostringstream oss;
    std::string pkey = pname + ":" + level;
    oss << getIndent(stage) << "[" << pkey << "](Rank: " << rank << ") " <<  format ;

    if (outFileName.compare("stdout") == 0) {
      (*fileptr) << logLevelToColor(pkey, oss.str()) << std::endl;
    } else {
      (*fileptr) << oss.str() << std::endl;
    }
  }
}

void Logger::log_c(VnV_Comm comm, std::string pname, std::string level,
                   std::string format, va_list args) {
  char buff[MAX_LOG_SIZE];
  int j = vsnprintf(buff, MAX_LOG_SIZE, format.c_str(), args);

  std::string message(buff);

  if (j > MAX_LOG_SIZE) {
    log(comm, pname, level,
        "Following message has been truncated due to buffer overflow");
  }
  log(comm, pname, level, message);
}

void Logger::addToBlackList(std::string packageName) {
  packageBlackList.insert(packageName);
}

void Logger::setLog(const std::string& filename) {
  outFileName = filename;
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
  } else if (filename.compare("engine") == 0) {
    fileptr = &std::cerr;  // Log to stderr until engine is configured
    engine = true;
  } else {
    std::ofstream fp;
    fp.open(filename, std::ofstream::out | std::ofstream::app);
    fileptr = &fp;
  }
}

void Logger::print() {
  VnV_Info(VNVPACKAGENAME, "Outfile Name: %s", outFileName.c_str());
}
