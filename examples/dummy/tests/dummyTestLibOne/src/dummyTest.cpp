#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV-Interfaces.h"

using namespace VnV;

class dummyTest : public ITest {
 public:
  TestStatus runTest(IOutputEngine* engine, int testStage, double* slope,
                     double* intersection) {
    // Write the slope and the intersection point to the  output file.
    engine->Put("slope", *slope);
    engine->Put("intersection", *intersection);
    return SUCCESS;
  }

  static void DeclareIO(IOutputEngine* engine) {
    engine->DefineDouble("slope");
    engine->DefineDouble("intersection");
  }

  void init() {
    m_parameters.insert(std::make_pair("slope", "double"));
    m_parameters.insert(std::make_pair("intersection", "double"));
  }

  TestStatus runTest(IOutputEngine* engine, int stage, NTV& parameters) {
    double* x = carefull_cast<double>(stage, "slope", parameters);
    double* y = carefull_cast<double>(stage, "intersection", parameters);
    int testStage = m_config.getStage(stage).getTestStageId();
    return runTest(engine, testStage, x, y);
  }
};

extern "C" {
ITest* dummyTest_maker() { return new dummyTest(); }

void dummyTest_DeclareIO(IOutputEngine* engine) {
  dummyTest::DeclareIO(engine);
}
};

class dummyTest_proxy {
 public:
  dummyTest_proxy() {
    VnV_registerTest("dummyTest", dummyTest_maker, dummyTest_DeclareIO);
  }
};

dummyTest_proxy p;

#endif
