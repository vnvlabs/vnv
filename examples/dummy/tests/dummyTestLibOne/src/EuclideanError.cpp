#ifndef _EuclideanError_H
#define _EuclideanError_H

#include <math.h>

#include <iostream>
#include <vector>

#include "VnV-Interfaces.h"

using namespace VnV;

class EuclideanError : public ITest {
 public:

    EuclideanError(TestConfig config) : ITest(config) {}

  TestStatus runTest(IOutputEngine* engine, int testStage,
                     std::vector<double>* measured,
                     std::vector<double>* exact) {
    if (measured->size() != exact->size()) {
      double m1 = -1;
      engine->Put("l2_error", m1);
      engine->Put("l1_error", m1);
      engine->Put("linf_error", m1);
      return FAILURE;
    }

    double l2(0), l1(0), linf(0);
    for (int i = 0; i < measured->size(); i++) {
      double diff = fabs((*measured)[i] - (*exact)[i]);
      l1 += diff;
      l2 += diff * diff;
      linf = (diff > linf) ? diff : linf;
    }

    l2 = sqrt(l2);
    engine->Put("l2_error", l2);
    engine->Put("l1_error", l1);
    engine->Put("linf_error", linf);
    return SUCCESS;
  }



  TestStatus runTest(IOutputEngine* engine, int stage, NTV& parameters) {
    std::vector<double>* x =
        carefull_cast<std::vector<double>>(stage, "measured", parameters);
    std::vector<double>* y =
        carefull_cast<std::vector<double>>(stage, "exact", parameters);
    int testStage = 0;//getTestStage(stage);
    return runTest(engine, testStage, x, y);
  }
};

ITest* EuclideanError_maker(TestConfig config) { return new EuclideanError(config); }

json EuclideanError_Declare() {
   return R"(
   {
           "name" : "EuclieanErorr",
           "title" : "Test For calculating euclidean error between two vectors.",
           "description" : "This test calculates the euclidean distance between two vectors",
           "expectedResult" : {"type" : "object"},
           "configuration" : {"type" : "object"},
           "parameters" : {
                 "measured" : "std::vector<double>",
                 "exact" : "std::vector<double>"
           },
           "requiredParameters" : ["measured","exact"],
           "io-variables" :{}
   })"_json;
}

class EuclideanError_proxy {
 public:
  EuclideanError_proxy() {
    VnV_registerTest("EuclideanError", EuclideanError_maker,
                     EuclideanError_Declare);
  }
};

EuclideanError_proxy euclideanError_proxy;

#endif
