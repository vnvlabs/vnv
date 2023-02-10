#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdio.h>

#include <cstdarg>
#include <exception>
#include <sstream>
#include <string>

namespace VnV {

struct VnVExceptionBase : public std::exception {
  std::string message;
  std::string function;
  std::string file;
  int line;

  VnVExceptionBase(std::string message);
  const char* what() const noexcept;

  VnVExceptionBase(std::string function, std::string file, int line, const char* format, ...);
  VnVExceptionBase(std::string function, std::string file, int line, std::string message);
};

// TODO differentiate this somehow -- This is called for things that are execptions
// that should never happen. They are not really asserts, because we want to catch
// then in production if they happen, but they are not really exceptions because they
// should never happen in the first place.
struct VnVBugReport : public VnVExceptionBase {
  VnVBugReport(std::string message) : VnVExceptionBase(message) {}
  VnVBugReport(std::string function, std::string file, int line, const char* format, ...);
};

namespace Exceptions {

VnVExceptionBase parseError(std::ifstream& fstream, long unsigned int byte, std::string message);

VnVExceptionBase fileReadError(std::string filename);

}  // namespace Exceptions

}  // namespace VnV

#define INJECTION_EXCEPTION(format, ...) VnV::VnVExceptionBase(__FUNCTION__, __FILE__, __LINE__, format, __VA_ARGS__)

#define INJECTION_EXCEPTION_(format) VnV::VnVExceptionBase(__FUNCTION__, __FILE__, __LINE__, format)

#define _INJECTION_EXCEPTION(format, ...) VnVExceptionBase(__FUNCTION__, __FILE__, __LINE__, format, __VA_ARGS__)

#define INJECTION_BUG_REPORT_(format) VnV::VnVBugReport(__FUNCTION__, __FILE__, __LINE__, format)

#define INJECTION_BUG_REPORT(format, ...) VnV::VnVBugReport(__FUNCTION__, __FILE__, __LINE__, format, __VA_ARGS__)

#ifdef NDEBUG
#  define INJECTION_ASSERT(...)
#else
#  define INJECTION_ASSERT(condition, format, ...)                                                   \
    if (!condition) {                                                                                \
      printf("Assert failed in function %s (file: %s, line %d\n", __FUNCTION__, __FILE__, __LINE__); \
      printf("Reason : " format, __VA_ARGS__);                                                       \
      std::terminate();                                                                              \
    }
#endif

// For debugging -- Anything that uses HTHROW has been handled (at least on this debugging pass. )
#define HTHROW throw

#endif  // EXCEPTIONS_H
