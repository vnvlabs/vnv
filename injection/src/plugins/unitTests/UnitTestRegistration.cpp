

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/IUnitTester.h"

/** Just list the names of any tests defined here
 * eg. VNV_INTERNAL_TESTS = X(name) X(test2) X(test3).
 *
 * Macro magic will do the rest
 */
#define VNV_INTERNAL_UNITTESTS \
  X(BFSTester)                 \
  X(Demo)                      \
  X(Sample2)

/** you should not need to touch anything below here. Unless your
 * test didn't use the INJECTION_TEST macro to declare it.
 */
#define X(name) DECLAREUNITTEST(name)
VNV_INTERNAL_UNITTESTS
#undef X

#define X(name) REGISTERUNITTEST(name)
namespace VnV {
namespace Registration {
void RegisterBuiltinUnitTests() { VNV_INTERNAL_UNITTESTS }
}  // namespace Registration
}  // namespace VnV
#undef VNV_INTERNAL_UNITTESTS
#undef X
