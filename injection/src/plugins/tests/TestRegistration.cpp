#include "json-schema.hpp"
#include "interfaces/ITest.h"


/** Just list the names of any tests defined here
 * eg. VNV_INTERNAL_TESTS = X(name) X(test2) X(test3).
 *
 * Macro magic will do the rest
 */
#define VNV_INTERNAL_TESTS X(provenance)


/** you should not need to touch anything below here. Unless your
 * test didn't use the INJECTION_TEST macro to declare it.
 */
#define X(name) DECLARETEST(name)
VNV_INTERNAL_TESTS
#undef X

#define X(name) REGISTERTEST(name)
namespace VnV {
 namespace Registration {
  void RegisterBuiltinTests() {
     VNV_INTERNAL_TESTS
  }
 }
}
#undef VNV_INTERNAL_TESTS
#undef X



