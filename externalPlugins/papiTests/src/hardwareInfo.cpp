
#include "VnV.h"
#include "papi.h"
#include <sys/utsname.h>
#include <iostream>

#define PNAME Papi


using namespace VnV;

namespace {

static bool PAPI_IS_INITIALIZED = false;

nlohmann::json getDefaultHWJson() {

   nlohmann::json j = nlohmann::json::object();
   const PAPI_hw_info_t* hwinfo = PAPI_get_hardware_info();

   if (hwinfo == nullptr) {
           std::cout << "sdfsdfsdfsdfsdf" << std::endl;
      j["error"] = "Cannot extract hardware information";
      return j;
   }

   j["version"] = PAPI_VERSION;

   struct utsname uname_info;
   uname(&uname_info);
   nlohmann::json uinfo = nlohmann::json::object();
   uinfo["Operating System"] = uname_info.sysname;
   uinfo["Release"] = uname_info.release;
   uinfo["Release"] = uname_info.nodename;
   uinfo["Release"] = uname_info.version;
   uinfo["Release"] = uname_info.machine;
   uinfo["Release"] = uname_info.version;
   j["Machine Info"] = uinfo;

   std::ostringstream oss;
   oss << hwinfo->vendor_string << "(" << hwinfo->vendor << ")";
   j["Vendor"] = oss.str();

   std::ostringstream os1;
   os1 << hwinfo->model_string << "(" << hwinfo->model << ")";
   j["Model"] = os1.str();

   j["CPU Revision"] = hwinfo->revision;
   if (hwinfo->cpuid_family > 0 ) {
       nlohmann::json fam = nlohmann::json::object();
       fam["family"] = hwinfo->cpuid_family;
       fam["Model"] = hwinfo->cpuid_model;
       fam["Stepping"] = hwinfo->cpuid_stepping;
       j["CPUID"] = fam;
   }
   j["CPU Max MHz"] = hwinfo->cpu_max_mhz;
   j["CPU Min MHz"] = hwinfo->cpu_min_mhz;
   j["Total Cores"] = hwinfo->totalcpus;
   j["SMT Threads Per Core"] = hwinfo->threads;
   j["Cores PER Socket"] = hwinfo->cores;
   j["Sockets"] = hwinfo->sockets;
   j["NUMA Regions"] = hwinfo->nnodes;
   j["Running in a VM"] = hwinfo->virtualized;
   j["VM Vendor"] = (hwinfo->virtualized) ? hwinfo->virtual_vendor_string : "N/A";
   j["VM Vendor Version"] = (hwinfo->virtualized) ? hwinfo->virtual_vendor_version : "N/A";

   int cnt = PAPI_get_opt(PAPI_MAX_HWCTRS, nullptr);
   j["Number Hardware Counters"] = cnt;
   int mpx = PAPI_get_opt(PAPI_MAX_MPX_CTRS,nullptr);
   j["Max multiplex Counters"] = mpx;

   PAPI_option_t options;
   PAPI_get_opt(PAPI_COMPONENTINFO, &options);
   unsigned int x= options.cmp_info->fast_counter_read;
   j["Fast Counter Read"] = x;
   return j;
}

bool InitalizePAPI() {
  if (!PAPI_IS_INITIALIZED) {
     int retVal = PAPI_library_init( PAPI_VER_CURRENT );
     PAPI_IS_INITIALIZED = (retVal == PAPI_VER_CURRENT);
  }
  return PAPI_IS_INITIALIZED;
 }
}

/**
 * COMPUTER HARDWARE AND MEMORY INFORMATION
 * ========================================
 *
 * The compute hardware information is:
 *
 * .. vnv-jchart::
 *    :main: Data.hardware.Value
 *
 *    $$main$$
 *
 */
INJECTION_TEST(PNAME,hardware_info) {
  if (InitalizePAPI() && (type == InjectionPointType::Begin || type == InjectionPointType::Single) ) {
    engine->Put( "hardware",getDefaultHWJson());
  }
  return SUCCESS;
}

class flopsRunner {
public:
  std::size_t EventSetCounter = 0;
  bool started = false;
  int EventSet = PAPI_NULL;
  std::vector<long long> values;

  flopsRunner() {
    InitalizePAPI();
    int r = PAPI_create_eventset(&EventSet);
    if ( r != PAPI_OK) {
        std::cout << PAPI_descr_error(r) << std::endl;;
      }
  }
  ~flopsRunner() {
    PAPI_destroy_eventset(&EventSet);
  }

  int addEvent(int id) {
    if (!started) {
       int r = PAPI_add_event(EventSet,id);
       if (r == PAPI_OK)
         EventSetCounter++;
       else
         std::cout << "SDFSDFSDF " << id << " " << PAPI_TOT_CYC << PAPI_descr_error(r) << std::endl;;
       return r;
    }
    return -1;
  }

  std::vector<long long>& start() {
    started = true;
    values.resize(EventSetCounter,0);
    int r = PAPI_start(EventSet);
    return values;
  }

  void reset() {
    std::fill(values.begin(),values.end(),0);
  }

  std::vector<long long>& stop() {
    started = false;
    PAPI_stop(EventSet, &values[0]);
    return values;
  }

  // Accum adds to values and then resets counters.
  // Read copies counters into values, does not reset.
  std::vector<long long>& iter( bool accum ) {
    if (accum) {
       PAPI_accum(EventSet, &values[0]);
    } else {
       PAPI_read(EventSet, &values[0]);
    }
    return values;
  }

};


/**
 * Recorded Floating point operations and cycles.
 * ==============================================
 *
 * The figure below shows the number of floating point operations recorded
 * using PAPI throughout the duration of this injection point.
 *
 *
 * .. vnv-chart::
 *    :labels: Data[?Name == 'stage'].Value
 *    :flops: Data[?Name == 'fpins'].Value
 *    :cycs: Data[?Name == 'cycles'].Value
 *
 *    {
 *       "type" : "line",
 *       "data" : {
 *          "labels" : $$labels$$,
 *          "datasets" : [
 *             {
 *               "label": "Recorded Floating Point Operations",
 *               "borderColor": "rgb(0, 255, 0)",
 *               "data": $$flops$$,
 *               "yAxisID" : "A"
 *             },
 *             {
 *               "label": "Recorded Cycles",
 *               "borderColor": "rgb(255, 0, 0)",
 *               "data": $$cycs$$,
 *               "yAxisID" : "B"
 *             }
 *          ]
 *       },
 *       "options" : {
 *           "scales" : {
 *             "yAxes" : [
 *                { "id" : "A" , "position" : "left", "type" : "linear" },
 *                { "id" : "B", "position" : "right", "type" : "linear" }
 *             ]
 *
 *           }
 *       }
 *    }
 *
 * .. note::
 *    Counters will include any cost associated with injection point tests in child nodes. Users should use caution when using nested profiling with this toolkit.
 *
 */
INJECTION_TEST_R(PNAME,flops, flopsRunner) {
  InitalizePAPI();
  if (type == InjectionPointType::Single) {
     engine->Put( "stage", stageId);
     engine->Put( "cycles", 0);
     engine->Put( "fpins",0);
     return SUCCESS;
  } else if (type == InjectionPointType::Begin) {
      runner->addEvent(PAPI_TOT_CYC);
      runner->addEvent(PAPI_FP_INS);
      runner->start();
  } else if ( type == InjectionPointType::End) {
      runner->stop();
  } else {
      runner->iter(true);
  }

  if (runner->values.size() > 0 ) {
        engine->Put( "cycles", (double) (runner->values)[0]);
        engine->Put( "fpins", (double) (runner->values)[1]);
        engine->Put( "stage", stageId);
  }

  //Reset --> This gets the change in cyles.
  runner->reset();

  return SUCCESS;
}














