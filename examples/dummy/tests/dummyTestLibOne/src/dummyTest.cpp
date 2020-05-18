#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV.h"
#include "interfaces/ITest.h"


INJECTION_TEST(dummyTest, double slope, double intersection ) {
   engine->Put(comm, "slope", get<double>("slope"));
   engine->Put(comm, "intersection", get<double>("intersection"));
   return SUCCESS;
}


#endif
