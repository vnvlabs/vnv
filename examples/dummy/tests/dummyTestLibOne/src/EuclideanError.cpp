#ifndef _EuclideanError_H
#define _EuclideanError_H

#include <math.h>

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/ITest.h"

using namespace VnV;

class EuclideanError : public ITest {
 public:

    EuclideanError(TestConfig config) : ITest(config) {}

  TestStatus runTest(VnV_Comm comm,OutputEngineManager* engine, int testStage,
                     std::vector<double> measured,
                     std::vector<double> exact) {
    if (measured.size() != exact.size()) {
      double m1 = -1;
      engine->Put(comm,"l2_error", m1);
      engine->Put(comm,"l1_error", m1);
      engine->Put(comm,"linf_error", m1);
      return FAILURE;
    }

    double l2(0), l1(0), linf(0);
    for (int i = 0; i < measured.size(); i++) {
      double diff = fabs(measured[i] - exact[i]);
      l1 += diff;
      l2 += diff * diff;
      linf = (diff > linf) ? diff : linf;
    }

    l2 = sqrt(l2);
    engine->Put(comm,"l2_error", l2);
    engine->Put(comm,"l1_error", l1);
    engine->Put(comm,"linf_error", linf);
    return SUCCESS;
  }



  TestStatus runTest(VnV_Comm comm,OutputEngineManager *engine, InjectionPointType type, std::string stageId) {
    GetRef(x,"measured",std::vector<double>);
    GetRef(y,"exact", std::vector<double>);
    return runTest(comm,engine, 0, x, y);
  }
};

ITest* euclideanError_maker(TestConfig config) { return new EuclideanError(config); }

json euclideanError_declare() {
   return R"(
   {
           "name" : "EuclieanError",
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


#endif
