
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>

#include "VnV.h"
#include "class1.h"
#include "class2.h"
#include "dlclass1.h"
#include "dlclass2.h"

#define SPNAME SampleExecutable

template <typename T> class f {
 public:
  T ff;
  T gg;
  int ggg;
  int getF(int t) {
    /** sdfsdfsdfsdf **/
    INJECTION_POINT(SPNAME, VSELF(SPNAME), sdfsdf, ff, gg, ggg);
    return 1;
  }
};

/**
 * Create a custom log level with stanadrd coloring.
 **/
INJECTION_LOGLEVEL(SPNAME, custom, )

/**
 * Let the VnV Toolkit know there is a subpackage linked to this executable.
 */
INJECTION_SUBPACKAGE(SPNAME, DummyLibOne)

INJECTION_TRANSFORM(SPNAME, sampleTransform, std::vector<double>, double) {
  return NULL;
}

/**
 * @brief INJECTION_TEST
 * @param vals
 */
INJECTION_TEST(SPNAME, sampleTest, std::vector<double> vals) {
  auto vals = get<std::vector<double>>("vals");
  for (auto& it : vals) {
    engine->Put( "Key", it);
    engine->Put( "Value", it);
  }
  return SUCCESS;
}

template <typename T, typename X> int templateFnc(int x, T y, X xx) {
  /**
   *  Template function evaluation
   *  ============================
   *
   *  An example of an injection point inside a template function. Template
   *  functions are interesting because it can be hard to map between injection
   *  points and tests in these cases.
   */
  INJECTION_POINT(SPNAME, VSELF(SPNAME), templateFn, x, y, xx);
  return 0;
}

class test1 {
 public:
  int function1(int x) {
    std::vector<double> samplePoints(10), samplePoints1(10), samplePoints3(13);
    /*
        Test1::function One
        ===================

        A simple looped injection point wrapping a for loop.
    */
    INJECTION_LOOP_BEGIN(SPNAME, VWORLD(SPNAME), Function1Class1, samplePoints,
                         samplePoints1, samplePoints3)
    for (int i = 0; i < 10; i++) {
      samplePoints.push_back(i);

      /**
       * Injection point stage Documentation
       * ===================================
       *
       * Injection point stage documentaiton is parsed by the parser, but
       * might not be used depending on the report generator. In either case,
       * it has the same access to the dat aelements of the overall injection
       * points.
       */
      INJECTION_LOOP_ITER(SPNAME, Function1Class1, inner)
      samplePoints1.push_back(i * i);
    }
    /**
       End Function1 Class 1
       =====================

       The function ended.
    */
    INJECTION_LOOP_END(SPNAME, Function1Class1)
    return 11;
  }
};

#include <map>

int function1(int x) {
  std::map<double, double> samplePoints;
  /**
      Another injection point
      =======================

      This is another injection point. This one is
      part of a global function.

  **/
  INJECTION_LOOP_BEGIN(SPNAME, VSELF(SPNAME), Function1, samplePoints)
  for (int i = 0; i < 10; i++) {
    samplePoints[i] = i;
    INJECTION_LOOP_ITER(SPNAME, Function1, inner)
  }
  INJECTION_LOOP_END(SPNAME, Function1)

  return 11;
}

static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";

/**
 * Options Documentation
 * =====================
 *
 * This is a quick blurb before the options command that will be used at some
 * point to aid in the development of input files.
 */
INJECTION_OPTIONS(SPNAME, schemaCallback) {}

// If compiled with MPI, then set the comm for this package to mpi.
#ifdef WITH_MPI
#  include <mpi.h>
INJECTION_EXECUTABLE(SPNAME, VNV, mpi)
#else
#  define MPI_Init(...)
#  define MPI_Finalize()
INJECTION_EXECUTABLE(SPNAME, VNV, serial)
#endif

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  /**
   * Sample Executable
   * =================
   *
   * This executable is designed to show a number of different injection points.
   * The first stage is to initialize the library. The text that appears in the
   * comment above the initialize call represents the introduction in the final
   * report.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv,
                       (argc == 2) ? argv[1] : "./vv-input.json");

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
  std::this_thread::sleep_for(std::chrono::seconds(2));
  //  f2(10, 10.0);

  double aa = 0.0;
  int min = 0;
  int max = 100;
  int count = 2;
  int i = min;

  INJECTION_POINT(SPNAME, VWORLD(SPNAME), functionTest, function1)

  /**
     Looped Injection Point with a Callback function
     ===============================================

     This is a looped injection point with a built in callback function.
     Callback functions are usefull as they allow data to be injected directly
     into the injection point description (this comment)

     In this loop, we iterate across a range [ :vnv:`Data.Data.min`,
  :vnv:`Data.Data.min`, :vnv:`Data.Data.max`) with

     a step of :vnv:`Data.Data.count`. At each step, the INJECTION_LOOP_ITER
  call is made, representing an internal stage of the injection point. This is
  turn calls the injection point call back, which logs the value of the
  injection point parameter "aa" (aa is a double set randomly in each step of
  the for loop). We plot aa against the step value using the chart directive.

     .. vnv-chart::
        :labels: Data.Data[?Name == 'x'].to_string(Value)
        :vals: Data.Data[?Name == 'y'].Value


        {
           "type" : "line",
           "data" : {
              "labels": $$labels$$,
              "datasets" : [
                 {
                  "label" : "A random Number",
                  "data": $$vals$$,
                  "fill" : true
                 }
              ]
           },
           "options" : {
              "responsive" : true,
              "title" : { "display" : true, "text" : "A sample Graph using the
  Chart directive" }, "yaxis" : { "display" : true, "scaleLabel" : {"display" :
  true, "labelString": "Value"} }
           }
        }



  **/
  INJECTION_LOOP_BEGIN_C(
      SPNAME, VSELF(SPNAME), loopTest,
      [](VnV_Comm comm, VnV::VnVParameterSet& p,
         VnV::OutputEngineManager* engine, VnV::InjectionPointType type,
         std::string stageId) {
        if (type == VnV::InjectionPointType::Iter) {
          const double& ab = p["aa"].getByRtti<double>();
          const int& i = p["i"].getByRtti<int>();
          engine->Put( "y", ab);
          engine->Put( "x", i);
        } else if (type == VnV::InjectionPointType::Begin) {
          /** Comment block in lambda function **/
          engine->Put( "min", p["min"].getByRtti<int>());
          engine->Put( "max", p["max"].getByRtti<int>());
          engine->Put( "count", p["count"].getByRtti<int>());
        }
      },
      aa, min, max, count, i);

  for (; i < max; i += count) {
    aa = 100 * ((double)rand() / (double)RAND_MAX);

    /** Testing stufff
     */
    INJECTION_LOOP_ITER(SPNAME, loopTest, internal)
  }
  /** sdfsdfsdfsdf**/
  INJECTION_LOOP_END(SPNAME, loopTest)

  sample_class_1.function1(10);
  sample_class_2.function1(10);
  sample_class_3.function1(10);
  sample_class_4.function1(10);

  /**
     Conclusion.
     ===========

     Whatever we put here will end up in the conclusion.
  */
  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
