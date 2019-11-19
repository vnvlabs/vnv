
#include <iostream>
#include <vector>

#include "VnV.h"

template <typename T>
class hello {
public:
int function_in_template(int x, T y) {
 std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);
 
 INJECTION_POINT(ErrorTest, 0, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
 }
};


class test1 {

int function_in_class(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_POINT(ErrorTest, 9999, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
}

};

int function_raw(int x) {
  std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

  INJECTION_POINT(ErrorTest, 0, std::vector<double>, samplePoints,
                  std::vector<double>, samplePoints1, std::vector<double>,
                  samplePoints3)
  return 11;
}

int main(int argc, char** argv) {
  VnV_init(&argc, &argv, "./sample.json");



  VnV_finalize();
}
