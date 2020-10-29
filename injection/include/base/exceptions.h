#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <sstream>
#include <string>

namespace VnV {

struct VnVExceptionBase : public std::exception {
  std::string message;

  VnVExceptionBase(std::string message);
  const char* what() const throw();

  VnVExceptionBase(const char* format, ...) {
    
  }
};

namespace Exceptions {



VnVExceptionBase parseError(std::ifstream& fstream, long unsigned int byte,
                            std::string message);

VnVExceptionBase fileReadError(std::string filename);

}  // namespace Exceptions

}  // namespace VnV

#endif  // EXCEPTIONS_H
