
/** @file provenance.cpp */

#ifndef vnv_cputimer_H
#define vnv_cputimer_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

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

  void setUnit(std::string unit) { this->unit = unit; }

  void start() { startTime = std::chrono::high_resolution_clock::now(); }

  template <typename T>
  auto durr(std::chrono::high_resolution_clock::time_point stop) {
    return std::chrono::duration_cast<T>(stop - startTime).count();
  }

  double split() {
    auto stop = std::chrono::high_resolution_clock::now();

    if (unit.compare("seconds") == 0) {
      return durr<std::chrono::seconds>(stop);
    } else if (unit.compare("milliseconds") == 0) {
      return durr<std::chrono::milliseconds>(stop);
    } else if (unit.compare("microseconds") == 0) {
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
 * The overall time was :vnv:`Data.TotalTime` :vnv:`Data.units`
 *
 * .. vnv-chart::
 *    :labels: Data[?TypeStr == 'Double'].Name
 *    :ydata: Data[?MetaData.tag == 'value'].Value
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
 *       },
 *       "options" : {
 *           "responsive" : true,
 *           "title" : { "display" : true, "text" : "CPU Time at the begining of each injection point." },
 *           "scales": {
 *               "yAxes": [{
 *                   "scaleLabel": {
 *                       "display": true,
 *                       "labelString": "CPU Time (nanoseconds)"
 *                   }
 *               }],
 *               "xAxes": [{
 *                   "scaleLabel": {
 *                       "display":true,
 *                       "labelString": "Injection Point Stage"
 *                   }
 *               }]
 *            }
 *        }
 *    }
 *
 */
INJECTION_TEST_RS(VNVPACKAGENAME, cputime, cpuRunner, cpuRunner::provSchema()) {
  if (type == InjectionPointType::Single) {
    VnV_Warn(VNVPACKAGENAME,
             "Attempt to time a non looped injection point. Returning zero for "
             "cputime");
    double c = 0;
    engine->Put( "Start", c, {{"tag","value"}});
    engine->Put( "TotalTime", c, {{"tag","value"}});
    
  } else if (type == InjectionPointType::Begin) {
    double cc = 0;
    const json& c = getConfigurationJson();
    auto it = c.find("units");
    if (it != c.end()) {
      runner->setUnit(it->get<std::string>());
    }
    engine->Put( "units", runner->unit);
    engine->Put( "Start", cc,{{"tag","value"}});
    runner->start();
  } else if (type == InjectionPointType::End) {
    engine->Put( "TotalTime", runner->split(),{{"tag","value"}});
  } else {
    engine->Put( stageId, runner->split(),{{"tag","value"}});
  }
  return SUCCESS;
}

#endif
