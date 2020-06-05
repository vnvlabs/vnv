

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/ISerializer.h"

/** Just list the names of any tests defined here
 * eg. VNV_INTERNAL_TESTS = X(name) X(test2) X(test3).
 *
 * Macro magic will do the rest
 */
#define VNV_INTERNAL_SERIALIZERS X(doubleToString)

/** you should not need to touch anything below here. Unless your
 * test didn't use the INJECTION_TEST macro to declare it.
 */
#define X(name) DECLARESERIALIZER(name)
VNV_INTERNAL_SERIALIZERS
#undef X

#define X(name) REGISTERSERIALIZER(name)

namespace VnV {
namespace Registration {
void RegisterBuiltinSerializers() { VNV_INTERNAL_SERIALIZERS }
}  // namespace Registration
}  // namespace VnV
#undef VNV_INTERNAL_SERIALIZERS
#undef X
