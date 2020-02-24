

#ifndef VV_VNV_H
#define VV_VNV_H

// VnV.h represents the include required to use the library at runtime. This includes all
// the functions required to initialize and register the library. It should be included and
// compilable using a C compiler to insure it can be used in C and C++.

// C Interface for the runtime functions.
#include "c-interfaces/RunTime.h"

// C Interface for the Injection points
#include "c-interfaces/Injection.h"

// C Interface for the Json Object. These functions allow C libraries to utilize the JSON
// input file options.
#include "c-interfaces/CJson.h"

// C Interface for the Logging components of VnV
#include "c-interfaces/Logging.h"

#endif // GAURD
