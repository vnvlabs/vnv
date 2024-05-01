
/** @file Logger.cpp Implementation of the Logger class as defined in
    base/Logger.h
**/
#include "base/Logger.h"  //Prototype.
#include <mpi.h>
#include <sstream>  //stringstream

#include "base/Runtime.h"                   // RunTime.
#include "base/stores/OutputEngineStore.h"  // OutputEngineStore
#include "common-interfaces/Logging.h"      // Logging macros
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

std::string VnV::Logger::logLevelToColor(std::string level, std::string message) {
  if (!RunTime::instance().useAsciiColors()) return message;

  auto it = logLevelsToColor.find(level);
  if (it != logLevelsToColor.end()) {
    std::ostringstream oss;
    oss << it->second << message << reset;
    return oss.str();
  }
  return message;
};

void Logger::up() { stage++; }
void Logger::down() { stage--; }

Logger::Logger() {
  registerLogLevel(VNVPACKAGENAME_S, "INFO", green, false);
  registerLogLevel(VNVPACKAGENAME_S, "DEBUG", green, false);
  registerLogLevel(VNVPACKAGENAME_S, "WARN", blue, true);
  registerLogLevel(VNVPACKAGENAME_S, "ERROR", red, true);
};

void Logger::registerLogLevel(std::string packageName, std::string name, std::string color, bool on) {
  logLevelsToColor[packageName + ":" + name] = color;
  logs[packageName + ":" + name] = on;
}

std::string Logger::getIndent(int stages) {
  std::string s = "";
  for (int i = 0; i < std::max(0, stages); i++) s += "----";
  return s;
}

void Logger::setLogLevel(std::string level, bool on) { 
  logs[level] = on;
}


void Logger::write_to_file(ICommunicator_ptr comm, std::string pname, int stage, std::string level, std::string message) {
  if (ltype != LogWriteType::NONE) {
    if (comm->Rank() == 0 ) {
        std::ostringstream oss;
        std::string pkey = pname + ":" + level;
        oss << getIndent(stage) << "[" << pkey << "](Comm: " << comm->uniqueId() <<"): " << message;

        if (ltype != LogWriteType::FILE) {
          (*fileptr) << logLevelToColor(pkey, oss.str()) << std::endl;
        } else {
          (*fileptr) << oss.str() << std::endl;
        }
    }
  }
}

void Logger::log(VnV_Comm comm, std::string pname, std::string level, std::string format, bool saved) {

  if (!locked || (engine && !OutputEngineStore::instance().isInitialized())) { 
    savedLogs.push(std::make_tuple(pname, stage, level, format, comm));
    return;
  }

  auto cc = CommunicationStore::instance().getCommunicator(comm);


  if (saved ) {
     while (savedLogs.size() > 0) {
        auto t = savedLogs.front();
        VnV_Comm cc = std::get<4>(t);
        log(cc, std::get<0>(t).c_str(), std::get<2>(t), std::get<3>(t), false);
        savedLogs.pop();

      }
  }

  if (packageBlackList.find(pname) != packageBlackList.end()) {
    return;
  }

 
  auto it = logs.find(pname + ":" + level);
  if (it != logs.end() && !it->second) {/*return*/}
      

  if (engine ) {
    try {
      OutputEngineManager* eng = OutputEngineStore::instance().getEngineManager();
      auto c = CommunicationStore::instance().getCommunicator(comm);
      eng->Log(c, pname.c_str(), stage, level, format);
    } catch (std::exception& e) {
      savedLogs.push(std::make_tuple(pname, stage, level, format, comm));
    }
  }

  auto c = CommunicationStore::instance().getCommunicator(comm);
  write_to_file(c,pname, stage, level, format);
}

void Logger::log_c(VnV_Comm comm, std::string pname, std::string level, std::string format, va_list args) {
  char buff[MAX_LOG_SIZE];
  int j = vsnprintf(buff, MAX_LOG_SIZE, format.c_str(), args);

  std::string message(buff);

  if (j > MAX_LOG_SIZE) {
    log(comm, pname, level, "Following message has been truncated due to buffer overflow");
  }
  log(comm, pname, level, message);
}

void Logger::addToBlackList(std::string packageName) { packageBlackList.insert(packageName); }

void Logger::setLog(bool useEngine, LogWriteType t, const std::string& filename) {
  outFileName = filename;
  if (locked) {
    if (fileptr != nullptr && fileptr != &std::cout && fileptr != &std::cerr) {
      dynamic_cast<std::ofstream*>(fileptr)->close();
    }
  }
  locked = true;
  this->ltype = t;
  this->engine = useEngine;


  if (t == LogWriteType::STDOUT) {
    fileptr = &std::cout;
  } else if (t == LogWriteType::STDERR) {
    fileptr = &std::cerr;
  } else if (t == LogWriteType::FILE) {
    std::ofstream fp;
    fp.open(filename, std::ofstream::out | std::ofstream::app);
    fileptr = &fp;
  } else {
  }
}


void Logger::print() { VnV_Info(VNVPACKAGENAME, "Outfile Name: %s", outFileName.c_str()); }
