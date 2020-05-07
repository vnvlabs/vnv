#include "json-schema.hpp"
#include "interfaces/ITest.h"

namespace VnV {
 namespace ProvenanceTest {
    ITest* maker(TestConfig config);
    json declare();
 }

 namespace Registration {
  void RegisterBuiltinTests() {
    VnV::registerTest("provenance",VnV::ProvenanceTest::maker, VnV::ProvenanceTest::declare);
  }
 }

}


