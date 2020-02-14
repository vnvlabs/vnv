
/** @file vv-logging.cpp **/

# include "base/vv-logging.h"

# include "VnV.h"
# include <fstream>
# include <iostream>
# include <stdio.h>
# include <stdarg.h>
# include <sstream>

# include "base/vv-output.h"
# include "base/vv-runtime.h"

using namespace VnV;

namespace {
constexpr const char *red = "\033[31m";
constexpr const char *green = "\033[32m";
constexpr const char *blue = "\033[34m";
constexpr const char *magenta = "\033[35m";
constexpr const char *yellow = "\033[33m";
constexpr const char *white = "\033[37m";
constexpr const char *cyan = "\033[36m";
constexpr const char *reset = "\033[0m";
}

std::string VnV::Logger::logLevelToColor(std::string level, std::string message) {
   if (!RunTime::instance().useAsciiColors())
       return message;

  auto it = logLevelsToColor.find(level);
  if ( it != logLevelsToColor.end()) {
      std::ostringstream oss;
      oss << it->second << message << reset;
      return oss.str();
  }
  return message;
};


Logger::Logger(){
    registerLogLevel("STAGE_START",yellow);
    registerLogLevel("STAGE_END", yellow);
    registerLogLevel("INFO", green);
    registerLogLevel("DEBUG", cyan);
    registerLogLevel("WARN", magenta);
    registerLogLevel("ERROR", red);
};

void Logger::registerLogLevel(std::string name, std::string color) {
    logLevelsToColor[name] = color;
}


std::string Logger::getIndent(int stage) {
    std::string s = "";
    for ( int i = 0; i < std::max(0,stage); i++) s+= "\t";
    return s;
}

void Logger::setLogLevel(std::string level, bool on) {
  logs.insert(std::make_pair(level, on));
}

void Logger::log(std::string pname, std::string level, std::string format) {
    if (!locked) return;
    if (packageBlackList.find(pname) != packageBlackList.end()) {
        return;
    }
    auto it = logs.find(level);
    if (it != logs.end() && !it->second) return;

    if ( engine ) {
       EngineStore::getEngineStore().getEngineManager()->getOutputEngine()->Log(pname.c_str(), stage.size(), level, format );
    } else {
        std::ostringstream oss;
        oss << "[" << pname << ":" << level << "] ";
        if ( outFileName.compare("stdout") == 0) {
            (*fileptr) << getIndent(stage.size()) << logLevelToColor(level,oss.str()) << format << std::endl;
        } else {
            (*fileptr) << getIndent(stage.size()) << oss.str() << format << std::endl;

        }
    }

 }



int Logger::beginStage(std::string pname, std::string format, va_list args) {
    log_c(pname,"STAGE_START", format, args);
    stage.push(refcount);
    return refcount++;
}

void Logger::endStage(int ref ) {
    if ( stage.size() == 0) return;

    int cStage = stage.top();
    while ( cStage != ref  )  {
        VnV_Warn("Incorrect stage name or missing StageEnd call %d (expected: %d)", ref, cStage);
        stage.pop();
        if ( stage.size() == 0 ) {
            break;
        } else {
            cStage = stage.top();
        }
      }
      if ( stage.size() > 0 )
         stage.pop();
      log("","STAGE_END", "");
}

void Logger::log_c(std::string pname, std::string level, std::string format, va_list args) {

      char buff[MAX_LOG_SIZE];
      int j = vsnprintf(buff,MAX_LOG_SIZE,format.c_str(), args);

      std::string message(buff);

      if ( j > MAX_LOG_SIZE) {
          log(pname, level, "Following message has been truncated due to buffer overflow");
      }
      log(pname,level,message);
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
    int a = VnV_BeginStage("Logger Configuration");
    VnV_Info("Outfile Name: %s", outFileName.c_str());
    VnV_EndStage(a);
}
