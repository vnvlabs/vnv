﻿

#ifndef VV_VNV_H
#define VV_VNV_H

// VnV.h represents the include required to use the library at runtime. This
// includes all the functions required to initialize and register the library.
// It should be included and compilable using a C compiler to insure it can be
// used in C and C++.

// C Interface for the runtime functions.
#include "c-interfaces/RunTime.h"

// Cpp Injection Includes. Allows specification of Injection points with Runtime
// type info
#ifdef __cplusplus

#  include "interfaces/CppInjection.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ISerializer.h"
#  include "interfaces/ITest.h"
#  include "interfaces/ITransform.h"
#  include "interfaces/IUnitTest.h"
#  include "interfaces/argType.h"
#  include "interfaces/ICommunicator.h"

#else
#  include "c-interfaces/Injection.h"
#endif

// C Interface for the Json Object. These functions allow C libraries to utilize
// the JSON input file options.
#include "c-interfaces/CJson.h"

// C Interface for defining the communicator.
#include "c-interfaces/Communication.h"

// C Interface for the Logging components of VnV
#include "c-interfaces/Logging.h"

#endif  // GAURD
