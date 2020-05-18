

#include "VnV.h"
#include "interfaces/ITest.h"

static json options_schema = R"({"type":"object"})"_json;

INJECTION_OPTIONS(options_schema) {
    VnV_Info("DummyTestLibrary options callback: %s", config.dump().c_str());
}

DECLARETEST(dummyTest)
DECLARETEST(EuclideanError)
INJECTION_REGISTRATION() {
    REGISTER_OPTIONS
    REGISTERTEST(dummyTest)
    REGISTERTEST(EuclideanError)
}



