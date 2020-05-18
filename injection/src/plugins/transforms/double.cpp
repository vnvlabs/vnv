
#ifndef Sample2_H
#define Sample2_H

#include "VnV.h"
#include "interfaces/ITransform.h"

#include <sstream>

INJECTION_TRANSFORM_R(doubleToInt, int, double, int) {
     *runner = *ptr;
     return runner.get();
}

#endif
