
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

namespace VnV {
namespace Engines {
    OutputEngineManager* DebugEngineBuilder();
    OutputEngineManager* JsonEngineBuilder();

#ifdef WITH_ADIOS
    OutputEngineManager* AdiosEngineBuilder();
#endif
}

namespace Registration {

   void RegisterBuiltinEngines() {
      // Register the engines.
      VnV::registerEngine("debug",VnV::Engines::DebugEngineBuilder);
      VnV::registerEngine("json", VnV::Engines::JsonEngineBuilder);
#ifdef WITH_ADIOS
      VnV::registerEngine("adios", VnV::Engines::AdiosEngineBuilder);
#endif
   }
}
}
