
/** @file provenance.cpp */

#ifndef vnv_cputimer_H
#define vnv_cputimer_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>

#include "base/DistUtils.h"
#include "c-interfaces/Logging.h"
#include "interfaces/ITest.h"

using namespace VnV;

namespace {

/**
 * This test is a test that checks the provenance of the executable. This test
 * does a FULL provenace tracking.
 */
class cpuRunner {
 public:

  std::vector<std::pair<std::string, long>> vals;
  std::chrono::high_resolution_clock::time_point startTime;
  cpuRunner() {}

  std::string unit = "nanoseconds";

  void setUnit(std::string unit) {
     this->unit = unit;
  }

  void start() {
      startTime = std::chrono::high_resolution_clock::now();
  }

  template<typename T>
  auto durr(std::chrono::high_resolution_clock::time_point stop) {
      return std::chrono::duration_cast<T>(stop- startTime).count();
  }

  double split() {
       auto stop = std::chrono::high_resolution_clock::now();

       if (unit.compare("seconds")==0) {
          return durr<std::chrono::seconds>(stop);
        } else if (unit.compare("milliseconds")==0) {
          return durr<std::chrono::milliseconds>(stop);
        } else if (unit.compare("microseconds")==0) {
          return durr<std::chrono::microseconds>(stop);
        } else {
          return durr<std::chrono::nanoseconds>(stop);
        }
  }

  static std::string provSchema() {
  return R"({
          "type":"object",
          "properties" : {
             "units" : {"type":"string", "enum":["seconds","milliseconds","microseconds","nanoseconds"] }
           },
           "additionalProperties" : false
           }
         )";
}

  virtual ~cpuRunner();
};

cpuRunner::~cpuRunner() {}

}  // namespace

/**
 * Injection Point Timing Results
 * ==============================
 *
 * The overall time was :vnv:`$.data.TotalTime` :vnv:`$.data.units`
 *
 * .. vnv-chart::
 *    :labels: $.data[?TypeStr == "Double"].Name
 *    :ydata: $.data[?TypeStr == "Double"].Value
 *
 *    {
 *       "type" : "line",
 *       "data" : {
 *          "labels" : $$labels$$,
 *          "datasets" : [{
 *             "label": "Recorded CPU Times",
 *             "backgroundColor": "rgb(255, 99, 132)",
 *             "borderColor": "rgb(255, 99, 132)",
 *             "data": $$ydata$$
 *           }]
 *       }
 *    }
 *
 */
INJECTION_TEST_RS(cputime, cpuRunner, cpuRunner::provSchema()) {
  if (type == InjectionPointType::Single) {
     VnV_Warn("Attempt to time a non looped injection point. Returning zero for cputime");
     long c=0;
     engine->Put(comm,"Start",c);
     engine->Put(comm, "TotalTime", c);
   } else if ( type == InjectionPointType::Begin ) {
     long cc = 0;
     const json& c = getConfigurationJson();
     auto it = c.find("units");
     if (it != c.end()) {
       runner->setUnit(it->get<std::string>());
     }
     engine->Put(comm, "units", runner->unit);
     engine->Put(comm, "Start", cc);
     runner->start();
   } else if ( type == InjectionPointType::End ) {

     std::cout << runner->split();
     engine->Put(comm, "TotalTime", runner->split());
   } else {
     engine->Put(comm, stageId, runner->split());
   }
   return SUCCESS;
}

#endif
