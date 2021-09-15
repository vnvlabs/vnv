
#include <iostream>
#include <memory>
/** Include the VnV include file **/
#include "VnV.h"

// Transforms are objects that map from one parameter type to another. One of
// the key components of assigning tests to injection points is mapping from the
// injection point parameters to the test parameters. To enable the development
// of generic tests, the toolkit includes an automatic parameter transformation
// function.

// This function performs a BFS of all available transforms to find the shortest
// valid path for transforming a parameter from one type to another. Users can
// add transforms to that list using the "transform" api

// Transform from a double to an int.
INJECTION_TRANSFORM_R(Samples, doubleToInt, int, double, int) {
  *runner = *ptr;
  return runner.get();
}
// Transform from an int to a double.
INJECTION_TRANSFORM_R(Samples, intToDouble, double, int, double) {
  *runner = *ptr;
  return runner.get();
}

// Some transform to fetch a member element.
class KSP {};
class PC {
 public:
  KSP ksp;
};

INJECTION_TRANSFORM(Samples, pcToKsp, PC, KSP) { return &(ptr->ksp); }

/// A poly morphic example
class B;

class A {
  std::string type;

 public:
  A(std::string t) : type(t) {}
  virtual void x() {}

  B* getAsB() {
    if (type == "B") {
      return (B*)this;
    }
    throw VnV::VnVExceptionBase("Invalid Conversion");
  }
};

class B : public A {
 public:
  B() : A("B") {}
};

INJECTION_TRANSFORM_R(Samples, BtoA, int, B, A) {
  return dynamic_cast<A*>(ptr);
}

INJECTION_TRANSFORM_R(Samples, AtoB, int, A, B) { return ptr->getAsB(); }
