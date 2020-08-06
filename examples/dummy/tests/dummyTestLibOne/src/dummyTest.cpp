#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV.h"
#include "interfaces/ITest.h"

#define DLT DummyTestLibOne

INJECTION_TEST(DLT, dummyTest, double slope, double intersection) {
  engine->Put(comm, "slope", get<double>("slope"));
  engine->Put(comm, "intersection", get<double>("intersection"));
  return SUCCESS;
}

#endif
