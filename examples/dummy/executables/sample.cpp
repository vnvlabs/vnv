
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>

#include "VnV.h"
#include "class1.h"
#include "class2.h"
#include "dlclass1.h"
#include "dlclass2.h"


template <typename T> class f {
 public:
  T ff;
  T gg;
  int ggg;
  int getF(int t) {
    INJECTION_POINT(VSELF, sdfsdf, ff, gg, ggg);
    return 1;
  }
};

/**
 * Create a custom log level with stanadrd coloring.
 **/
INJECTION_LOGLEVEL(custom, )

/**
 * Let the VnV Toolkit know there is a subpackage linked to this executable.
 */
INJECTION_SUBPACKAGE(DummyLibOne)

INJECTION_TRANSFORM(sampleTransform, std::vector<double>, double) {
  return NULL;
}

/**
 * @brief INJECTION_TEST
 * @param vals
 */
INJECTION_TEST(sampleTest, std::vector<double> vals) {
  auto vals = get<std::vector<double>>("vals");
  for (auto& it : vals) {
    engine->Put(comm, "Key", it);
    engine->Put(comm, "Value", it);
  }
  return SUCCESS;
}

template <typename T, typename X> int templateFnc(int x, T y, X xx) {
  INJECTION_POINT(VSELF, templateFn, x, y, xx);
  return 0;
}

class test1 {
 public:
  int function1(int x) {
    std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);

    INJECTION_LOOP_BEGIN(VWORLD, Function1Class1, samplePoints,
                         samplePoints1, samplePoints3)
    for (int i = 0; i < 10; i++) {
      samplePoints.push_back(i);
      INJECTION_LOOP_ITER(Function1Class1, inner)
      samplePoints1.push_back(i * i);
    }

    INJECTION_LOOP_END(Function1Class1)
    return 11;
  }
};

#include <map>

int function1(int x) {
  std::map<double, double> samplePoints;

  INJECTION_LOOP_BEGIN(VSELF, Function1, samplePoints)
  for (int i = 0; i < 10; i++) {
    samplePoints[i] = i;
    INJECTION_LOOP_ITER(Function1, inner)
  }
  INJECTION_LOOP_END(Function1)

  return 11;
}

static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

INJECTION_OPTIONS(schemaCallback) {}

// If compiled with MPI, then set the comm for this package to mpi.
#ifdef WITH_MPI
   #include <mpi.h>
   INJECTION_COMM(mpi)
#else
  #define MPI_Init(...)
  #define MPI_Finalize()
#endif



int main(int argc, char** argv) {

  MPI_Init(&argc,&argv);

  INJECTION_INITIALIZE(&argc, &argv, (argc == 2) ? argv[1] : "./vv-input.json");

  function1(10);

  f<double> why;
  why.getF(1);

  f<int> hh;
  hh.getF(1);

  struct xx {
    int y;
  };

  templateFnc(1, 1.0, "sdfsdf");
  templateFnc(1, "", "");

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

  MPI_Finalize();
}
