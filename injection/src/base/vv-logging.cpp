
/** @file vv-logging.cpp **/

#  include "vv-logging.h"

# include "VnV.h"
#  include <fstream>
#  include <iostream>
#  include <stdio.h>
#  include <stdarg.h>
# include "vv-output.h"

using namespace VnV;

std::string VnV::logLevelToString(LogLevel level) {
  switch (level) {
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO";
  case LogLevel::WARN:
    return "WARN";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::STAGE_START:
    return "BEGIN STAGE";
  case LogLevel::STAGE_END:
    return "END STAGE";
  }
  throw "enum added to LogLevel without implementing to string method [see vv-logging.cpp]";
};

Logger::Logger(){};

LogLevel VnV::logLevelFromString(std::string level) {
  if (level.compare("DEBUG") == 0) return LogLevel::DEBUG;
  if (level.compare("INFO") == 0) return LogLevel::INFO;
  if (level.compare("WARN") == 0) return LogLevel::WARN;
  if (level.compare("ERROR") == 0) return LogLevel::ERROR;
  if (level.compare("BEGIN STAGE") == 0) return LogLevel::STAGE_START;
  if (level.compare("END STAGE") == 0) return LogLevel::STAGE_END;
  throw "Invalid";
}

std::string VnV::getIndent(int stage) {
    std::string s = "";
    for ( int i = 0; i < std::max(0,stage); i++) s+= "\t";
    return s;
}

void Logger::setLogLevel(LogLevel level, bool on) {
  logs.insert(std::make_pair(level, on));
}

void Logger::log(std::string pname, LogLevel level, std::string format) {
    if (!locked) return;
    if (packageBlackList.find(PACKAGE_NAME) != packageBlackList.end()) {
        return;
    }
    auto it = logs.find(level);
    if (it != logs.end() && !it->second) return;

    if ( engine ) {
       EngineStore::getEngineStore().getEngineManager()->getOutputEngine()->Log(pname.c_str(), stage, level, format );
    }
    (*fileptr) << getIndent(stage) << "[" << pname << ":" << logLevelToString(level) << "] " << format
               << std::endl;
}

void Logger::log_c(std::string pname, LogLevel level, std::string format, va_list args) {
      char buff[MAX_LOG_SIZE];
      int j = vsnprintf(buff,MAX_LOG_SIZE,format.c_str(), args);

      std::string message(buff);

      if ( j > MAX_LOG_SIZE) {
          log(pname, level, "Following message has been truncated due to buffer overflow");
      }
      if ( level == LogLevel::STAGE_END) {
         stage--;
      }
      log(pname,level, message);
      if ( level == LogLevel::STAGE_START) {
          stage++;
      }
}

void Logger::addToBlackList(std::string packageName){
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
    fileptr = nullptr;
    engine = true;
  } else {
    std::ofstream fp;
    fp.open(filename, std::ofstream::out | std::ofstream::app);
    fileptr = &fp;
  }
}

void Logger::print() {
    VnV_BeginStage("Logger Configuration");
    VnV_Info("Outfile Name: %s", outFileName.c_str());
    VnV_EndStage("");
}
