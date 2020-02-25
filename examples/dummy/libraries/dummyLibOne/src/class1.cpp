
#include "VnV.h"
#include "dlclass1.h"

using namespace dummyLibOne;

int class1::function1(int x) {
  double slope = 2;
  double intersection = 3;
  double value = 0;

  auto a = VnV_BeginStage("An example of a third party library using the VnV logging capabilities");

  INJECTION_LOOP_BEGIN(Hello_temp_sub,slope,intersection,x,value);

  value = slope * x + intersection;

  INJECTION_LOOP_END(Hello_temp_sub);

  VnV_EndStage(a);

  return static_cast<int>(value);
}

REGISTER_VNV() {
    // This code will be generated automatically when using the clang libTool.
    VnV_Debug("Inside the Registration Callback for the DummyLibOne");
    // This is where all the injection points should be registered.

    const char* config1 = R"(
  {
   "name" : "class2_function1",
   "parameters" : {
      "x" : "int"
    }
  }
)";

Register_Injection_Point(config1);

const char* config = R"(
{
   "name" : "Hello_temp_sub",
   "parameters" : {
      "slope" : "double",
      "intersection" : "double",
      "x" : "int",
      "value" : "double"
    }
}
)";

Register_Injection_Point(config);

}

