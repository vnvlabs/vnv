#include "base/exceptions.h"
#include <cstdio>

#define ERRORMESSAGEMAXSIZE 2048

VnV::VnVExceptionBase::VnVExceptionBase(std::string message) {
  this->message = message;
}

const char* VnV::VnVExceptionBase::what() const throw() {
  return message.c_str();
}
