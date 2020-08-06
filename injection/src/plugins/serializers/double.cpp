
#ifndef Sample2_H
#define Sample2_H

#include "VnV.h"
#include "interfaces/ISerializer.h"

INJECTION_SERIALIZER(VNVPACKAGENAME, doubleToString, double) {
  return std::to_string(*ptr);
}

#endif
