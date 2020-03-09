

#include "VnV.h"
#include "interfaces/ITest.h"

VnV::ITest* euclideanError_maker(VnV::TestConfig config);
json euclideanError_declare();

VnV::ITest* dummyTest_maker(VnV::TestConfig config);
json dummyTest_declare();

static json options_schema = R"({"type":"object"})"_json;


void options_callback(json &callbackJson) {
    VnV_Info("DummyTestLibrary options callback: %s", callbackJson.dump().c_str());
}


REGISTER_VNV() {

    // Register some options.
    VnV_Register_Options(options_schema, options_callback );

    // Register the tests.
    VnV::registerTest("EuclideanError", euclideanError_maker,euclideanError_declare);
    VnV::registerTest("DummyTest", dummyTest_maker,dummyTest_declare);


}


