﻿

#ifndef VNV_H
#define VNV_H

// VnV.h represents the include required to use the library at runtime. This
// includes all the functions required to initialize and register the library.
// It should be included and compilable using a C compiler to insure it can be
// used in C and C++.


#ifdef __cplusplus
#  include "interfaces/IOptions.h"

#  include "interfaces/ICommunicator.h"
#  include "interfaces/IDataType.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/IReduction.h"
#  include "interfaces/ISampler.h"
#  include "interfaces/ITest.h"
#  include "interfaces/IUnitTest.h"
#  include "interfaces/IWorkflow.h"
#  include "interfaces/Initialization.h"
#  include "interfaces/argType.h"
#  include "interfaces/points/Injection.h"
#  include "interfaces/IAction.h"

#else

#  include "c-interfaces/Initialization.h"
#  include "c-interfaces/Options.h"
#  include "c-interfaces/points/Injection.h"

#endif

// C Interface for the runtime functions.
#include "common-interfaces/RunTime.h"

// C Interface for defining the communicator.
#include "common-interfaces/Communication.h"

// C Interface for the Logging components of VnV
#include "common-interfaces/Logging.h"


#endif  // GAURD
