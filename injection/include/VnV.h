

#ifndef VV_VNV_H
#define VV_VNV_H

// C Interface for the runtime functions.
#include "c-interfaces/runtime-interface.h"

// C Interface for the Injection points
#include "c-interfaces/injection-point-interface.h"

// C Interface for the Json Object. These functions allow C libraries to utilize the JSON
// input file options.
#include "c-interfaces/json-interface.h"

// C Interface for the Logging components of VnV
#include "c-interfaces/logging-interface.h"

#if __cplusplus

 # include "interfaces/ioutputengine.h"
 # include "interfaces/itransform.h"
 # include "interfaces/iserializer.h"
 # include "interfaces/itest.h"
 # include "interfaces/iunittester.h"

#endif //CPLUS_PLUS

#endif // GAURD
