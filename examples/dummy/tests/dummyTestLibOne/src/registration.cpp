

#include "VnV.h"


VnV::ITest* euclideanError_maker(VnV::TestConfig config);
json euclideanError_declare();

VnV::ITest* dummyTest_maker(VnV::TestConfig config);
json dummyTest_declare();

char* options_schema() {
    return R"({"type":"object"})";
}

void options_callback(c_json callbackJson) {
    json j = VnV::asJson(callbackJson);
    VnV_Info("DummyTestLibrary options callback: %s", j.dump().c_str());
}


REGISTER_VNV_CALLBACK() {

    // Register some options.
    VnV_Register_Options(options_schema, options_callback );

    // Register the tests.
    VnV::registerTest("EuclideanError", euclideanError_maker,euclideanError_declare);
    VnV::registerTest("DummyTest", dummyTest_maker,dummyTest_declare);


}


