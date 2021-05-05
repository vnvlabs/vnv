#ifndef DLCLASS1_H
#define DLCLASS1_H
#include "VnV.h"

#define DLPNAME DummyLibOne

namespace dummyLibOne {

class class1 {
 public:
  int function1(int x);
  int function2(int y) {
    INJECTION_POINT(VNV_STR(DLPNAME), VSELF, "function_in_header", y);

    return 0;
  }
};

}  // namespace dummyLibOne

#endif
