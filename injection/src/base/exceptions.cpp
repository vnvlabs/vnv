#include "base/exceptions.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <queue>

#define ERRORMESSAGEMAXSIZE 2048

VnV::VnVExceptionBase::VnVExceptionBase(std::string message) {
  this->message = message;
  this->function = "<unknown>";
  this->file = "<unknown>";
  this->line = -1;
}

const char* VnV::VnVExceptionBase::what() const throw() {
  return message.c_str();
}

VnV::VnVExceptionBase::VnVExceptionBase(std::string function, std::string file, int line, std::string message) {
  this->function = function;
  this->file = file;
  this->line = line;
  this->message = message;
 
}

namespace {
  std::string extract(const char* format, va_list args) {
    
    char buffer[250];
    int r = vsnprintf(buffer, sizeof buffer, format, args);

    const size_t len = r;
    if (len < sizeof buffer) {
      return {buffer, len};
    } else {
      return "There was an error writing the error";
   }
  }
}

VnV::VnVExceptionBase::VnVExceptionBase(std::string function, std::string file, int line, const char* format, ...) {
  this->function = function;
  this->file = file;
  this->line = line;
  
  va_list args;
  va_start(args, format);
  message = extract(format,args);
  va_end(args);

}

VnV::VnVBugReport::VnVBugReport(std::string function, std::string file, int line, const char* format, ...) : VnVExceptionBase("") {
  this->function = function;
  this->file = file;
  this->line = line;
  
  va_list args;
  va_start(args, format);
  message = extract(format,args);
  va_end(args);

}



VnV::VnVExceptionBase VnV::Exceptions::parseError(std::ifstream& fstream,
                                                  unsigned long byte,
                                                  std::string message) {
  std::ostringstream oss;
  oss << "Could not parse input File. The Json Error message was: \n"
      << "\t" << message << "\n\n";
  oss << "The Offending Json was \n...\n\n";

  fstream.clear();
  std::queue<std::string> lines;
  fstream.seekg(0, std::ios::beg);
  int count = 0;
  int after = -1;
  for (std::string line; std::getline(fstream, line);) {
    std::ostringstream iss;
    count++;
    unsigned long b = fstream.tellg();
    if (lines.size() == 9) lines.pop();

    if (b > byte && after < 0) {
      iss << count << "|>>> " << line << "\n";
      lines.push(iss.str());
      after = count;
    } else {
      iss << count << "|||| " << line << "\n";
      lines.push(iss.str());
    }
    if (after >= 0 && count > after + 3) {
      break;
    }
  }
  while (lines.size()) {
    oss << lines.front();
    lines.pop();
  }

  oss << "\n...\n\nSome common problems include trailing commas, unquoted keys "
         "and unmatched brackets\n\n";
  return VnVExceptionBase(oss.str());
}

VnV::VnVExceptionBase VnV::Exceptions::fileReadError(std::string filename) {
  std::ostringstream oss;
  oss << "Could not open file with name " << filename;
  return VnVExceptionBase(oss.str());
}
