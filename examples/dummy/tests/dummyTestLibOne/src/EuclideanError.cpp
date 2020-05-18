#ifndef EuclideanError_H
#define EuclideanError_H

#include <math.h>

#include <iostream>
#include <vector>

#include "VnV.h"
#include "interfaces/ITest.h"

using namespace VnV;

INJECTION_TEST(EuclideanError, std::vector<double> measured, std::vector<double> exact) {

    const std::vector<double>* measured = get<std::vector<double>>("measured");
    const std::vector<double>* exact = get<std::vector<double>>("exact");

    if (measured->size() != exact->size()) {
      double m1 = -1;
      engine->Put(comm,"l2_error", m1);
      engine->Put(comm,"l1_error", m1);
      engine->Put(comm,"linf_error", m1);
      return FAILURE;
    }

    double l2(0), l1(0), linf(0);
    for (std::size_t i = 0; i < measured->size(); i++) {
      double diff = fabs((*measured)[i] - (*exact)[i]);
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


#endif
