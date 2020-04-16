
/**
  @file ITransform.cpp
**/
#include "interfaces/ITransform.h"
#include "base/exceptions.h"

using namespace VnV;

void* ITransform::Transform(std::string from, std::string to, void* ptr, std::string &rtti ) {

    if ( from.compare(to) == 0 ) {
        rtti = ""; // Don't know the rtti of this one.
        return ptr;
    } else {
        throw VnVExceptionBase("Input Type Does not match output type when using Default Transform.");
    }
}


ITransform::ITransform(){};
ITransform::~ITransform(){};
