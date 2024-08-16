
/** @file provenance.cpp */

#ifndef vnv_cputimer_H
#define vnv_cputimer_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "shared/DistUtils.h"
#include "common-interfaces/all.h"
#include "interfaces/ITest.h"

using namespace VnV;

namespace {

/**
 * This test is a test that checks the provenance of the executable. This test
 * does a FULL provenance tracking.
 */
class cpuRunner {
 public:
  std::vector<std::pair<std::string, long>> vals;
  std::chrono::high_resolution_clock::time_point startTime;
  cpuRunner() {}

  std::string unit = "nanoseconds";

  void setUnit(std::string unit) { this->unit = unit; }

  void start() { startTime = std::chrono::high_resolution_clock::now(); }

  template <typename T> auto durr(std::chrono::high_resolution_clock::time_point stop) {
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
 * @title Injection Point Timing Results
 *
 * The overall time was :vnv:`TotalTime[0]` :vnv:`units[0]`
 *
 * .. vnv-chart::
 *
 *    {
 *       "type" : "line",
 *       "data" : {
 *          "labels" : {{as_json(Labels)}},
 *          "datasets" : [{
 *             "label": "Recorded CPU Times",
 *             "backgroundColor": "rgb(57, 105, 160)",
 *             "borderColor": "rgb(57, 105, 160)",
 *             "data": {{as_json(Data)}}
 *           }, {
 *             "label": "DT",
 *             "backgroundColor": "rgb(57, 105, 160)",
 *             "borderColor": "rgb(57, 105, 160)",
 *             "data": {{as_json(vec_delta(Data))}}
 *           }
 *       ]},
 *       "options" : {
 *           "animation" : false,
 *           "responsive" : true,
 *           "title" : { "display" : true,
 *                       "text" : "CPU Time at the begining of each injection point."
 *                     },
 *          "scales": {
 *             "yAxes": [{
 *               "scaleLabel": {
 *                 "display": true,
 *                 "labelString": "CPU Time ({{units[0]}})"
 *               }
 *            }],
 *            "xAxes": [{
 *              "scaleLabel": {
 *                 "display":true,
 *                 "labelString": "Injection Point Stage"
 *               }
 *            }]
 *          }
 *       }
 *    }
 *
 *
 */
INJECTION_TEST_RS(VNVPACKAGENAME, cputime, cpuRunner, cpuRunner::provSchema()) {
  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    const json& c = getConfigurationJson();
    auto it = c.find("units");
    if (it != c.end()) {
      runner->setUnit(it->get<std::string>());
    }
    engine->Put("units", runner->unit);
    runner->start();
  } else if (type == InjectionPointType::End) {
    engine->Put("TotalTime", runner->split(), {{"tag", "value"}});
  }

  engine->Put("Labels", stageId);
  engine->Put("Data", runner->split());
  return SUCCESS;
}

#endif
