
#include <mpi.h>
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
    INJECTION_POINT(VNV_STR(SPNAME), VSELF, "sdfsdf", ff, gg, ggg);
    return 1;
  }
};

/**
 * Create a custom log level with stanadrd coloring.
 **/
INJECTION_LOGLEVEL(VNV_STR(SPNAME), custom, )

/**
 * Let the VnV Toolkit know there is a subpackage linked to this executable.
 */
INJECTION_SUBPACKAGE(SPNAME, DummyLibOne)

INJECTION_TRANSFORM(SPNAME, sampleTransform, std::vector<double>, double) { return NULL; }

/**
 * Sample Test As Part of an executable
 * ====================================
 *
 *
 *
 *
 */
INJECTION_TEST(SPNAME, sampleTest, std::vector<double> vals) {
  auto vals = get<std::vector<double>>("vals");
  for (auto& it : vals) {
    engine->Put("Key", it);
    engine->Put("Value", it);
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
  INJECTION_POINT(VNV_STR(SPNAME), VSELF, "templateFn", x, y, xx);

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
    INJECTION_LOOP_BEGIN(VNV_STR(SPNAME), VWORLD, "Function1Class1", samplePoints, samplePoints1, samplePoints3);
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
      INJECTION_LOOP_ITER(VNV_STR(SPNAME), "Function1Class1", "inner");
      samplePoints1.push_back(i * i);
    }
    /**
       End Function1 Class 1
       =====================

       The function ended.
    */
    INJECTION_LOOP_END(VNV_STR(SPNAME), "Function1Class1");
    return 11;
  }
};

#include <map>

double func(double x) { return x * x * x - x * x + 2; }

// Derivative of the above function which is 3*x^x - 2*x
double derivFunc(double x) { return 3 * x * x - 2 * x; }

void newtonRaphson(double x, double eps, int rank) {
  double f = func(x);
  double fp = derivFunc(x);
  double h = f / fp;
  double iter = 0;

  /**
   *  Newton Raphson Method for finding the root of a function
   *  ========================================================
   *
   *  This function uses the NewtonRaphson method to solve
   *  the function
   *
   *  .. math::
   *
   *     f(x) = x^3 - x^2 + 2 = 0.
   *
   *  where the formula for the Newton Raphson method is:
   *
   *  .. math::
   *
   *     x_{n+1} = x_{n} + \frac{f(x_n)}{f'(x_n)}.
   *
   *  The solution is x = -1.
   *
   *  The newton raphson method is expected to converge quadratically to
   *  the root. The following chart shows the current value of the best
   *  guess at each iteration of the algorithm.
   *
   *  .. vnv-chart::
   *     :labels: Data.Data[?Name == 'iter'].to_string(Value)
   *     :vals: Data.Data[?Name == 'root'].Value
   *
   *
   *     {
   *        "type" : "line",
   *        "data" : {
   *           "labels": $$labels$$,
   *           "datasets" : [
   *              {
   *                "label" : "Approximate Root",
   *                "data": $$vals$$,
   *                "fill" : true,
   *                "backgroundColor": "rgb(255, 99, 132)",
   *                "borderColor": "rgb(255, 99, 132)"
   *              }
   *           ]
   *        },
   *        "options" : {
   *           "responsive" : true,
   *           "title" : { "display" : true,
   *              "text" : "Convergence of the Newton Raphson Method" },
   *              "scales": {
   *                 "yAxes": [
   *                   { "scaleLabel": {
   *                     "display": true,
   *                     "labelString": "Current Guess"
   *                   }
   *                 }],
   *                 "xAxes": [{
   *                   "scaleLabel": {
   *                     "display":true,
   *                     "labelString": "Iteration Number"
   *                   }
   *                  }]
   *             }
   *        }
   *     }
   *
   **/
  INJECTION_LOOP_BEGIN_C(
      VNV_STR(SPNAME), VSELF, "NewtonRaphson",
      [](VnV_Comm comm, VnV::VnVParameterSet& p, VnV::OutputEngineManager* engine, VnV::InjectionPointType type,
         std::string stageId) {
        const double& x = p["x"].getByRtti<double>();
        const double& iter = p["iter"].getByRtti<double>();
        const int& rank = p["rank"].getByRtti<int>();

        engine->Put("Rank", rank);

        engine->Put("root", x);
        engine->Put("iter", iter);
      },
      x, iter, rank);

  while (std::abs(h) >= eps) {
    f = func(x);
    fp = derivFunc(x);
    h = f / fp;
    x = x - h;
    iter++;
    INJECTION_LOOP_ITER(VNV_STR(SPNAME), "NewtonRaphson", "iteration");
  }
  INJECTION_LOOP_END(VNV_STR(SPNAME), "NewtonRaphson");
}

int function1(int x) {
  std::map<double, double> samplePoints;
  /**
      Another injection point
      =======================

      This is another injection point. This one is
      part of a global function.

  **/
  INJECTION_LOOP_BEGIN(VNV_STR(SPNAME), VSELF, "Function1", samplePoints);
  for (int i = 0; i < 10; i++) {
    samplePoints[i] = i;
    INJECTION_LOOP_ITER(VNV_STR(SPNAME), "Function1", "inner");
  }
  INJECTION_LOOP_END(VNV_STR(SPNAME), "Function1");

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

INJECTION_EXECUTABLE(SPNAME)

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /**
   * Sample Executable
   * =================
   *
   * This executable is designed to show a number of different injection points.
   * The first stage is to initialize the library. The text that appears in the
   * comment above the initialize call represents the introduction in the final
   * report.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, (argc == 2) ? argv[1] : "./vv-input.json");

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

  double start = -10 * (rank + 1);
  newtonRaphson(start, 0.000001, rank);

  INJECTION_POINT(VNV_STR(SPNAME), VWORLD, "functionTest", function1);

  std::cout << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << std::endl;

  MPI_Comm comm, comm1;
  MPI_Comm_split(MPI_COMM_WORLD, rank % 2, rank, &comm);
  MPI_Comm_split(MPI_COMM_WORLD, rank / 2, rank, &comm1);
  /**
   *
   *  Looped Injection Point with a Callback function
   *  ===============================================
   *
   *  This is a looped injection point with a built in callback function.
   *  Callback functions are usefull as they allow data to be injected directly
   *  into the injection point description (this comment)
   *
   *  In this loop, we iterate across a range [ :vnv:`Data.Data.min`,
   *  :vnv:`Data.Data.max`] with a step of :vnv:`Data.Data.count`. At each step,
   *  the INJECTION_LOOP_ITER call is made, representing an internal stage of
   *  the injection point. This is turn calls the injection point call back,
   *  which logs the value of the injection point parameter "aa"
   *  (aa is a double set randomly in each step of the for loop). We plot aa
   *  against the step value using the chart directive.
   *
   *  .. vnv-chart::
   *     :labels: Data.Data[?Name == 'x'].to_string(Value)
   *     :vals: Data.Data[?Name == 'y'].Value
   *
   *
   *     {
   *        "type" : "line",
   *        "data" : {
   *           "labels": $$labels$$,
   *           "datasets" : [
   *              {
   *               "label" : "A random Number",
   *               "data": $$vals$$,
   *               "fill" : true
   *              }
   *           ]
   *        },
   *        "options" : {
   *           "responsive" : true,
   *           "title" : { "display" : true,
   *                       "text" : "A sample Graph using the Chart directive"
   *           },
   *           "yaxis" : { "display" : true,
   *                       "scaleLabel" : {
   *                          "display" : true,
   *                          "labelString": "Value"
   *                       }
   *           }
   *        }
   *     }
   *
   **/
  INJECTION_LOOP_BEGIN_C(
      VNV_STR(SPNAME), VMPI(comm), "loopTest1",
      [](VnV_Comm comm, VnV::VnVParameterSet& p, VnV::OutputEngineManager* engine, VnV::InjectionPointType type,
         std::string stageId) {
        if (type == VnV::InjectionPointType::Iter) {
          const double& ab = p["aa"].getByRtti<double>();
          const int& i = p["i"].getByRtti<int>();
          engine->Put("y", ab);
          engine->Put("x", i);
        } else if (type == VnV::InjectionPointType::Begin) {
          /** Comment block in lambda function **/
          engine->Put("min", p["min"].getByRtti<int>());
          engine->Put("max", p["max"].getByRtti<int>());
          engine->Put("count", p["count"].getByRtti<int>());
        }
      },
      aa, min, max, count, i);

  for (; i < max; i += count) {
    aa = 100 * ((double)rand() / (double)RAND_MAX);

    /** Testing stufff
     */
    INJECTION_LOOP_ITER(VNV_STR(SPNAME), "loopTest1", "internal");
  }
  /** sdfsdfsdfsdf**/
  INJECTION_LOOP_END(VNV_STR(SPNAME), "loopTest1");

  INJECTION_LOOP_BEGIN_C(
      VNV_STR(SPNAME), VMPI(comm1), "loopTest2",
      [](VnV_Comm comm, VnV::VnVParameterSet& p, VnV::OutputEngineManager* engine, VnV::InjectionPointType type,
         std::string stageId) {
        if (type == VnV::InjectionPointType::Iter) {
          const double& ab = p["aa"].getByRtti<double>();
          const int& i = p["i"].getByRtti<int>();
          engine->Put("y", ab);
          engine->Put("x", i);
        } else if (type == VnV::InjectionPointType::Begin) {
          /** Comment block in lambda function **/
          engine->Put("min", p["min"].getByRtti<int>());
          engine->Put("max", p["max"].getByRtti<int>());
          engine->Put("count", p["count"].getByRtti<int>());
        }
      },
      aa, min, max, count, i);

  for (; i < max; i += count) {
    aa = 100 * ((double)rand() / (double)RAND_MAX);

    /** Testing stufff
     */
    INJECTION_LOOP_ITER(VNV_STR(SPNAME), "loopTest2", "internal");
  }
  /** sdfsdfsdfsdf**/
  INJECTION_LOOP_END(VNV_STR(SPNAME), "loopTest2");

  INJECTION_LOOP_BEGIN_C(
      VNV_STR(SPNAME), VSELF, "loopTest",
      [](VnV_Comm comm, VnV::VnVParameterSet& p, VnV::OutputEngineManager* engine, VnV::InjectionPointType type,
         std::string stageId) {
        if (type == VnV::InjectionPointType::Iter) {
          const double& ab = p["aa"].getByRtti<double>();
          const int& i = p["i"].getByRtti<int>();
          engine->Put("y", ab);
          engine->Put("x", i);
        } else if (type == VnV::InjectionPointType::Begin) {
          /** Comment block in lambda function **/
          engine->Put("min", p["min"].getByRtti<int>());
          engine->Put("max", p["max"].getByRtti<int>());
          engine->Put("count", p["count"].getByRtti<int>());
        }
      },
      aa, min, max, count, i);

  for (; i < max; i += count) {
    aa = 100 * ((double)rand() / (double)RAND_MAX);

    /** Testing stufff
     */
    INJECTION_LOOP_ITER(VNV_STR(SPNAME), "loopTest", "internal");
  }
  /** sdfsdfsdfsdf**/
  INJECTION_LOOP_END(VNV_STR(SPNAME), "loopTest");

  INJECTION_INPUT_FILE_(SPNAME, "IMAGE", VWORLD, "/home/ben/Downloads/mpi.jpeg", "image");
  INJECTION_INPUT_FILE_(SPNAME, "CSV", VWORLD, "/home/ben/te/test.csv", "csv");
  INJECTION_INPUT_FILE_(SPNAME, "GLVIS", VWORLD, "/home/ben/Downloads/laghos.saved", "glvis");
  INJECTION_INPUT_FILE_(SPNAME, "GLVIS", VWORLD, "/home/ben/Downloads/head-ascii.vti", "vti");
  INJECTION_INPUT_FILE_(SPNAME, "HTML", VWORLD, "/home/ben/Downloads/index.html", "html");
  INJECTION_INPUT_FILE_(SPNAME, "RST", VWORLD, "/home/ben/Downloads/sample.rst", "rst");
  INJECTION_INPUT_FILE_(SPNAME, "MARK", VWORLD, "/home/ben/Downloads/sample.md", "markdown");
  INJECTION_INPUT_FILE_(SPNAME, "CODE", VWORLD, "/home/ben/Downloads/sample.cpp", "code");
  INJECTION_INPUT_FILE_(SPNAME, "CODE", VWORLD, "/home/ben/Downloads/act_gpu.pdf", "pdf");

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
