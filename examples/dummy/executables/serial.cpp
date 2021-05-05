
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>

#include "VnV.h"


#define SPNAME SerialExample


static const char* schemaCallback = "{\"type\": \"object\", \"required\":[]}";
/**
 * Options Documentation
 * =====================
 *
 * This is a quick blurb before the options command that will be used at some
 * point to aid in the development of input files.
 */
INJECTION_OPTIONS(SPNAME, schemaCallback) {}
INJECTION_EXECUTABLE(SPNAME, VNV, serial)

int main(int argc, char** argv) {

  /**
   * Sample Executable
   * =================
   *
   * This executable is designed to show a number of different injection points.
   * The first stage is to initialize the library. The text that appears in the
   * comment above the initialize call represents the introduction in the final
   * report.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv,(argc == 2) ? argv[1] : "./vv-input.json");

  /**
     Conclusion.
     ===========

     Whatever we put here will end up in the conclusion.
  */
  INJECTION_FINALIZE(SPNAME);
}
