#ifndef _dummyTest_H
#define _dummyTest_H

#include "VnV.h"
#include "interfaces/ITest.h"

#define DLT DummyTestLibOne

INJECTION_TEST(DLT, dummyTest) {
  
  auto slope = GetRef("slope",double);
  auto intersection = GetRef("intersection",double);
          
  engine->Put("slope", slope);
  engine->Put("intersection", intersection);
  return SUCCESS;
}

#endif
