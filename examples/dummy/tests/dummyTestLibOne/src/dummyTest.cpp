#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV.h"
#include "interfaces/ITest.h"

using namespace VnV;

class dummyTest : public ITest {
 public:

    dummyTest(TestConfig config) : ITest(config) {

    }


    TestStatus runTest(VnV_Comm comm,OutputEngineManager *engine, int testStage, double slope,
                     double intersection) {

    std::string s;
    // Write the slope and the intersection point to the  output file.
    engine->Put(comm,"slope", slope);
    engine->Put(comm,"intersection", intersection);
    return SUCCESS;
  }

  TestStatus runTest(VnV_Comm comm, OutputEngineManager* engine, InjectionPointType type, std::string stageId) {
    GetRef(x,"slope",double);
    GetRef(y,"intersection",double);
    return runTest(comm, engine, 0, x, y);
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
