﻿

#ifndef VV_VNV_H
#define VV_VNV_H

// VnV.h represents the include required to use the library at runtime. This
// includes all the functions required to initialize and register the library.
// It should be included and compilable using a C compiler to insure it can be
// used in C and C++.

// C Interface for the runtime functions.
#include "c-interfaces/RunTime.h"

// C Interface for defining the communicator.
#include "c-interfaces/Communication.h"

// C Interface for the Logging components of VnV
#include "c-interfaces/Logging.h"

// C Interfaces for the options api.
#  include "c-interfaces/CJson.h"


#ifdef __cplusplus

#  include "interfaces/points/Injection.h"
#  include "interfaces/points/Iteration.h"
#  include "interfaces/points/Plug.h"

#  include "interfaces/ICommunicator.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ISerializer.h"
#  include "interfaces/ITest.h"
#  include "interfaces/ITransform.h"
#  include "interfaces/IUnitTest.h"
#  include "interfaces/IAction.h"
#  include "interfaces/IIterator.h"
#  include "interfaces/IPlug.h"
#  include "interfaces/argType.h"

#else

#  include "c-interfaces/points/Injection.h"
#  include "c-interfaces/points/Iteration.h"
#  include "c-interfaces/points/Plug.h"

#endif


#endif  // GAURD
