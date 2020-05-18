

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/ITransform.h"

/** Just list the names of any tests defined here
 * eg. VNV_INTERNAL_TESTS = X(name) X(test2) X(test3).
 *
 * Macro magic will do the rest
 */
#define VNV_INTERNAL_TRANSFORMS \
  X(doubleToInt)

/** you should not need to touch anything below here. Unless your
 * test didn't use the INJECTION_TEST macro to declare it.
 */
#define X(name) DECLARETRANSFORM(name)
VNV_INTERNAL_TRANSFORMS
#undef X

#define X(name) REGISTERTRANSFORM(name)
namespace VnV {
 namespace Registration {
  void RegisterBuiltinTransforms() {
     VNV_INTERNAL_TRANSFORMS
  }
 }
}
#undef VNV_INTERNAL_TRANSFORMS
#undef X
