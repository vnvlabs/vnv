

#include "VnV-Interfaces.h"
#include "VnV.h"


VnV::ITest* euclideanError_maker(VnV::TestConfig config);
json euclideanError_declare();

VnV::ITest* dummyTest_maker(VnV::TestConfig config);
json dummyTest_declare();

json options_schema() {
    return R"({"type":"object"})"_json;
}
void options_callback(json &callbackJson) {
    VnV_Info("DummyTestLibrary options callback: %s", callbackJson.dump().c_str());
}


REGISTER_VNV_CALLBACK() {

    // Register some options.
    VnV_registerOptions("DummyTestLibrary", options_schema, options_callback );

    // Register the tests.
    VnV_registerTest("EuclideanError", euclideanError_maker,euclideanError_declare);
    VnV_registerTest("DummyTest", dummyTest_maker,dummyTest_declare);


}


