﻿
#ifndef Sample2_H
#define Sample2_H

#include "VnV.h"
#include "interfaces/ITransform.h"

#include <sstream>

INJECTION_TRANSFORM_R(doubleToInt, double, double, int) {
     *runner = *ptr;
     return runner.get();
}

#endif
