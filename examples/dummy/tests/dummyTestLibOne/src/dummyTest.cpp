#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV-Interfaces.h"

using namespace VnV;

class dummyTest : public ITest {
 public:

    dummyTest(TestConfig config) : ITest(config) {

    }


    TestStatus runTest(IOutputEngine* engine, int testStage, double* slope,
                     double* intersection) {
    // Write the slope and the intersection point to the  output file.
    engine->Put("slope", *slope);
    engine->Put("intersection", *intersection);
    return SUCCESS;
  }

  TestStatus runTest(IOutputEngine* engine, int stage, NTV& parameters) {
    double* x = carefull_cast<double>(stage, "slope", parameters);
    double* y = carefull_cast<double>(stage, "intersection", parameters);
    int testStage = getTestStage(stage);
    return runTest(engine, testStage, x, y);
  }
};

ITest* dummyTest_maker(TestConfig config) { return new dummyTest(config); }

json dummyTest_Declare() {
    return R"(
{
  "name" : "dummyTest",
  "title" : "Dummy test for plotting a line.",
  "description" : "This test writes data for the equation of a line.",
  "expectedResult" : {"type" : "object"},
  "configuration" : {"type" : "object"},
  "stages" : {
     "-1" : {
        "slope" : "double",
        "intersection" : "double",
     }
  },
  "requiredStages" : ["-1"],
  "io-variables" : {}
}
)"_json;
}


class dummyTest_proxy {
 public:
  dummyTest_proxy() {
    VnV_registerTest("dummyTest", dummyTest_maker, dummyTest_Declare);
  }
};

dummyTest_proxy p;

#endif
