
#ifndef Sample2_H
#define Sample2_H

#include <sstream>

#include "VnV.h"
#include "interfaces/ITransform.h"

INJECTION_TRANSFORM_R(VNVPACKAGENAME, doubleToInt, int, double, int) {
  *runner = *ptr;
  return runner.get();
}

#endif
