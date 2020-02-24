
#include "VnV.h"
#include "dlclass1.h"

using namespace dummyLibOne;

int class1::function1(int x) {
  double slope = 2;
  double intersection = 3;
  double value = 0;

  auto a = VnV_BeginStage("An example of a third party library using the VnV logging capabilities");

  INJECTION_LOOP_BEGIN(Hello_temp_sub,double,slope,double,intersection,double,x,double,value);

  value = slope * x + intersection;

  VnV_Info("Some Additional information from this third party package.");
  VnV_Debug("Third party packages should define the PACKAGE_NAME macro inside the VnV.h file when compiling. This"
            "ensures the logging statements of the packages are correctly identified. In this case, the packageName"
            "should be DummyLibOne");

  INJECTION_LOOP_END(Hello_temp_sub,double,slope,double,intersection,double,x,double,value);

  VnV_EndStage(a);

  return (int) value;
}

REGISTER_VNV() {
    // This code will be generated automatically when using the clang libTool.
    VnV_Debug("Inside the Registration Callback for the DummyLibOne");
    // This is where all the injection points should be registered.

    const char* config = R"(
{
   "name" : "Hello_temp_sub",
   "type" : "LOOP",
   "package" : "DummyLibOne",
   "stages" : {
       "BEGIN" : {
          "parameters" : {
                "slope" : {
                    "class" : "double"
                },
                "intersection" : {
                    "class" : "double"
                }
          }
       },
       "END" : {
          "parameters" : {
                "slope" : {
                    "class" : "double"
                },
                "intersection" : {
                    "class" : "double"
                }
          }
       }
   }
})";

Register_Injection_Point("Hello_temp_sub", config);

}

