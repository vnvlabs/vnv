
#include "interfaces/IOutputEngine.h"

/**
 * All builtin engines should be registered here. All you need to do to register
 * an engine is:
 *
 * 1. declare the builder function (either include the header file, or
 * just forward delcare the function).
 * 2. register the engine in the RegisterBuiltinEngines function.
 * 3. Add the subdirectory in the CMake file.
 */

// Adios engine is optional (depends on if adios is installed. Only
// register it is with_adios is set.
#ifdef WITH_ADIOS
  #define ADIOSENGINE X(adios)
#else
  #define ADIOSENGINE
#endif

#define VNV_INTERNAL_ENGINES \
  ADIOSENGINE \
  X(json) \
  X(debug)

/** you should not need to touch anything below here. Unless your
 * test didn't use the INJECTION_ macro to declare it.
 */
#define X(name) DECLAREENGINE(name)
VNV_INTERNAL_ENGINES
#undef X

#define X(name) REGISTERENGINE(name)
namespace VnV {
 namespace Registration {
  void RegisterBuiltinEngines() {
     VNV_INTERNAL_ENGINES
  }
 }
}
#undef VNV_INTERNAL_ENGINES
#undef X

