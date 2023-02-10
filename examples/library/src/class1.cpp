
#include "VnV.h"
#include "dlclass1.h"

using namespace dummyLibOne;
#define DLPNAME DummyLibOne

int class1::function1(int x) {
  double slope = 2;
  double intersection = 3;
  double value = 0;

  INJECTION_LOOP_BEGIN(DLPNAME, VSELF, Hello_temp_sub, VNV_NOCALLBACK, slope, intersection, x, value);

  value = slope * x + intersection;

  INJECTION_LOOP_END(DLPNAME, Hello_temp_sub, VNV_NOCALLBACK);

  return static_cast<int>(value);
}
int class1::function2(int y) {
  INJECTION_POINT(DLPNAME, VSELF, function_in_header, VNV_NOCALLBACK, y);
  return 0;
}

// This is an example of how an application can provide initialization
// functions for VnV to use in applications using the library.
INJECTION_LIBRARY_NOCLANG(DummyApplication, DLPNAME);

void DummyVnV::Initialize(int argc, char** argv, const char* filename) {
  INJECTION_INITIALIZE(DummyApplication, &argc, &argv, filename);
}

void DummyVnV::Finalize() { INJECTION_FINALIZE(DummyApplication); }
