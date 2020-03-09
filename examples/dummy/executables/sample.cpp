
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
        INJECTION_POINT(sdfsdf, ff, gg, ggg );
        return 1;
    }
};



template <typename T, typename X>
int templateFnc(int x, T y, X xx) {
    INJECTION_POINT(templateFn,x,y,xx);
    return 0;
}


class test1 {

int function1(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_LOOP_BEGIN(Function1Class1, samplePoints, samplePoints1, samplePoints3)
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

  INJECTION_LOOP_BEGIN(Function1, samplePoints, samplePoints1, samplePoints3)
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(Function1, inner)
    samplePoints1.push_back(i * i);
  }
  INJECTION_LOOP_END(Function1)

  return 11;
};

// Write the injection point registation -- we use the array form to specify multiple points
// in a single string.
static const std::string injectionPoints = R"(
   [
       {
          "name" : "Function1",
          "parameters" : { "samplePoints" : "std::vector<double>", "samplePoints1" : "std::vector<double>" , "samplePoints3" : "std::vector<double>" }
       },
       {
          "name" : "Function1Class1",
          "parameters" : { "samplePoints" : "std::vector<double>", "samplePoints1" : "std::vector<double>" , "samplePoints3" : "std::vector<double>" }
       },
       {
          "name" : "Function_In_Template",
          "parameters" : { "samplePoints" : "std::vector<double>", "samplePoints1" : "std::vector<double>" , "samplePoints3" : "std::vector<double>" }
       }
   ]
)";

void callback() {
   // Here is where we would register all the injection points.
   VnV_Debug("Inside the Executable Call Back.");
   Register_Injection_Point(injectionPoints);
};

int main(int argc, char** argv) {

  VnV_init(&argc, &argv, (argc==2) ? argv[1] : "./vv-input.json", callback);

  function1(10);
 
  f<double> why;
  why.getF(1);

  f<int> hh;
  hh.getF(1);

  struct xx{
      int y;
  };
  xx ten;

  templateFnc(1,1.0, "sdfsdf");
  templateFnc(1,"","");

 // hello<int> whyint;
 // whyint.function1(1,1.0);

  class1 sample_class_1;
  class2 sample_class_2;

  dummyLibOne::class1 sample_class_3;
  dummyLibOne::class1 sample_class_4;

//  f2(10, 10.0);

  sample_class_1.function1(10);
  sample_class_2.function1(10);
  sample_class_3.function1(10);
  sample_class_4.function1(10);

  VnV_finalize();
}
