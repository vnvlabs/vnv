#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV.h"
#include "interfaces/itest.h"

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

  TestStatus runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId, std::map<std::string,void*> &parameters) {

    double* x = static_cast<double*>(parameters["slope"]);
    double* y = static_cast<double*>(parameters["intersection"]);
    return runTest(engine, 0, x, y);
  }
};

ITest* dummyTest_maker(TestConfig config) { return new dummyTest(config); }

json dummyTest_declare() {
    return R"(
{
  "name" : "dummyTest",
  "title" : "Dummy test for plotting a line.",
  "description" : "This test writes data for the equation of a line.",
  "expectedResult" : {"type" : "object"},
  "configuration" : {"type" : "object"},
  "parameters" : {
        "slope" : "double",
        "intersection" : "double"
   },
   "requiredParameters" : ["slope","intersection"],
  "io-variables" : {}
}
)"_json;
}

#endif
