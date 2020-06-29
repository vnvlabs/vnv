#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>
#include <sstream>

namespace VnV {

struct VnVExceptionBase : public std::exception {
  std::string message;

  VnVExceptionBase(std::string message);
  const char* what() const throw();
};

namespace  Exceptions {

VnVExceptionBase parseError(std::ifstream &fstream, long unsigned int byte, std::string message );

VnVExceptionBase fileReadError(std::string filename);


}



}  // namespace VnV

#endif  // EXCEPTIONS_H
