
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

Logger::Logger() {
  registerLogLevel(VNVPACKAGENAME_S, "STAGE_START", yellow);
  registerLogLevel(VNVPACKAGENAME_S, "STAGE_END", yellow);
  registerLogLevel(VNVPACKAGENAME_S, "INFO", green);
  registerLogLevel(VNVPACKAGENAME_S, "DEBUG", cyan);
  registerLogLevel(VNVPACKAGENAME_S, "WARN", magenta);
  registerLogLevel(VNVPACKAGENAME_S, "ERROR", red);
};

void Logger::registerLogLevel(std::string packageName, std::string name,
                              std::string color) {
  logLevelsToColor[packageName + ":" + name] = color;
}

std::string Logger::getIndent(int stage) {
  std::string s = "";
  for (int i = 0; i < std::max(0, stage); i++) s += "\t";
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
      // Next statement throws if true.
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
      eng->Log(c, pname.c_str(), stage.size(), level, format);
    } catch (...) {
      // Logging statements that occur prior to the engine being configured at
      // written to std::out.
      if (savedLogs.size() > MAXSAVED_LOGS) {
        std::cout << "To Many Logs before engine configuration: Dumping to "
                     "stdout instead"
                  << std::endl;
        auto& t = savedLogs.front();
        std::ostringstream oss;
        oss << "[" << std::get<0>(t) << ":" << std::get<2>(t) << "] ";
        std::cout << getIndent(std::get<1>(t))
                  << logLevelToColor(std::get<2>(t), oss.str())
                  << std::get<3>(t) << std::endl;
        savedLogs.pop();
      }
      savedLogs.push(std::make_tuple(pname, stage.size(), level, format, comm));
    }
  } else {
    std::ostringstream oss;
    oss << "[" << pname << ":" << level << "] ";
    if (outFileName.compare("stdout") == 0) {
      (*fileptr) << getIndent(stage.size()) << logLevelToColor(level, oss.str())
                 << format << std::endl;
    } else {
      (*fileptr) << getIndent(stage.size()) << oss.str() << format << std::endl;
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
