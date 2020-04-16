#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

namespace VnV {

struct VnVExceptionBase : public std::exception
{
    std::string message;
    VnVExceptionBase(std::string message);
    const char * what () const throw ();
};

}

#endif // EXCEPTIONS_H
