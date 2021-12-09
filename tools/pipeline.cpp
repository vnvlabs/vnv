
#include <iostream>
#include <vector>
#define RPNAME pipeline
#include "VnV.h"

INJECTION_EXECUTABLE(RPNAME)

/// Set up some options for the executable.
json conf;
INJECTION_OPTIONS(RPNAME, R"({
  "type" : "object",
  "properties" : {
    "package" : {"type":"string", "default" : "VNV" },
    "name" : {"type":"string", "default" : "identity" },
    "filename" : {"type":"string", "default" : "" },
    "stdout" : {"type":"boolean", "default" : false },
    "config" : {"type" : "object" } 
  }
})") {
  conf = config;
}

int main(int argc, char** argv) {
  INJECTION_INITIALIZE(RPNAME, &argc, &argv, argv[1]);

  // Extract the options
  int stdo = conf.value("stdout", false) ? 1 : 0;
  std::string file = conf.value("filename", "");
  std::string package = conf.value("package", "VNV");
  std::string name = conf.value("name", "identity");
  std::string conff = conf.value("config", json::object()).dump();

  // Generate the pipeline
  VnV_generate_pipeline(package.c_str(), name.c_str(), conff.c_str(), file.c_str(), stdo);

  // Finalize
  INJECTION_FINALIZE(RPNAME)
}
