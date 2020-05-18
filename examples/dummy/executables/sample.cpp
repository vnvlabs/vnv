
#include <iostream>
#include <vector>

#include "VnV.h"
#include "class1.h"
#include "class2.h"
#include "dlclass1.h"
#include "dlclass2.h"
#include <typeinfo>

template<typename T>
class f {
public:
    T ff;
    T gg;
    int ggg;
    int getF(int t) {
        INJECTION_POINT(VnV_Comm_Self, sdfsdf, ff, gg, ggg );
        return 1;
    }
};



template <typename T, typename X>
int templateFnc(int x, T y, X xx) {
    INJECTION_POINT(VnV_Comm_Self,templateFn,x,y,xx);
    return 0;
}


class test1 {
public:
int function1(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_LOOP_BEGIN(VnV_Comm_Self, Function1Class1, samplePoints, samplePoints1, samplePoints3)
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(Function1Class1, inner)
    samplePoints1.push_back(i * i);
  }

  INJECTION_LOOP_END(Function1Class1)
  return 11;
}

};

int function1(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_LOOP_BEGIN(VnV_Comm_Self, Function1, samplePoints, samplePoints1, samplePoints3)
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(Function1, inner)
    samplePoints1.push_back(i * i);
  }
  INJECTION_LOOP_END(Function1)

  return 11;
};

namespace {
static const std::string params = R"({
                                     "samplePoints" :  "std::vector<double>",
                                     "samplePoints1" : "std::vector<double>",
                                     "samplePoints3" : "std::vector<double>"
                                  })";
}

INJECTION_REGISTRATION() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back.");
   Register_Injection_Point("Function1Class1", params);
   Register_Injection_Point("Function_In_Template", params);
   Register_Injection_Point("Function1", params);
};

int main(int argc, char** argv) {

  INJECTION_INITIALIZE(&argc, &argv, (argc==2) ? argv[1] : "./vv-input.json");

  function1(10);

  f<double> why;
  why.getF(1);

  f<int> hh;
  hh.getF(1);

  struct xx{
      int y;
  };

  templateFnc(1,1.0, "sdfsdf");
  templateFnc(1,"","");

 // hello<int> whyint;
 // whyint.function1(1,1.0);

  test1 a;
  a.function1(10);

  class1 sample_class_1;
  class2 sample_class_2;

  dummyLibOne::class1 sample_class_3;
  dummyLibOne::class1 sample_class_4;

//  f2(10, 10.0);

  sample_class_1.function1(10);
  sample_class_2.function1(10);
  sample_class_3.function1(10);
  sample_class_4.function1(10);

  INJECTION_FINALIZE();
}
