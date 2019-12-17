
#include <iostream>
#include <vector>

#include "VnV.h"
#include "class1.h"
#include "class2.h"
#include "dlclass1.h"
#include "dlclass2.h"

template <typename T>
class hello {
public:
int function1(int x, T y) {
 std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);
 
 INJECTION_POINT(Function_In_Teplate, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
 }
};


class test1 {

int function1(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_LOOP_BEGIN(Function1Class1, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(Function1Class1, inner, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
    samplePoints1.push_back(i * i);
  }

  INJECTION_LOOP_END(Function1Class1, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
}

};

int function1(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_LOOP_BEGIN(Function1, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  for (int i = 0; i < 10; i++) {
    samplePoints.push_back(i);
    INJECTION_LOOP_ITER(Function1, inner, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
    samplePoints1.push_back(i * i);
  }

  INJECTION_LOOP_END(Function1, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
};

void callback() {
   VnV_Debug("Inside the Executable Call Back.");
};

int main(int argc, char** argv) {
  VnV_init(&argc, &argv, "./sample.json",callback);

  function1(10);
 
  hello<double> why;
  why.function1(1,1.0);


  class1 sample_class_1;
  class2 sample_class_2;

  dummyLibOne::class1 sample_class_3;
  dummyLibOne::class1 sample_class_4;

  sample_class_1.function1(10);
  sample_class_2.function1(10);
  sample_class_3.function1(10);
  sample_class_4.function1(10);

  VnV_finalize();
}
