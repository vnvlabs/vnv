#include "base/exceptions.h"

VnV::VnVExceptionBase::VnVExceptionBase(std::string message) {
  this->message = message;
}

const char* VnV::VnVExceptionBase::what() const throw() {
  return message.c_str();
}
