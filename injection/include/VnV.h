

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

// TODO: Decide on including these interfaces in the VnV.h header include for all C++ projects. This would be fine,
// but, the interfaces include a header only json library. We could forward declare the json class to remove that dependency,
// but, then the user would be forced to include the "json.hpp" when implementing any plugin. At that point, its probably more
// intuitive to foce the user to include the interface/i*.h header instead.
#if __cplusplus

 # include "interfaces/ioutputengine.h"
 # include "interfaces/itransform.h"
 # include "interfaces/iserializer.h"
 # include "interfaces/itest.h"
 # include "interfaces/iunittester.h"

#endif //CPLUS_PLUS

#endif // GAURD
