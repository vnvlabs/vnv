/** @file provenance.cpp */

#ifndef _provenance_H
#define _provenance_H

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
class provenanceRunner {
 public:
  provenanceRunner() {}

  TestStatus getProvHistory(ICommunicator_ptr comm, IOutputEngine* engine, int argc,
                            char** argv, json configFile, json& inputFiles) {
    {
      // Add the current working directory
      std::string currentWorkingDirectory(DistUtils::getCurrentDirectory());
      engine->Put( "cwd", currentWorkingDirectory);
    }

    {
      // Add the command line
      std::string commandline = argv[0];
      for (int i = 1; i < argc; i++) {
        std::string v(argv[i]);
        commandline += " " + std::string(argv[i]);
      }
      VnV::MetaData m;
      m.insert(std::make_pair("tag", "hello"));
      engine->Put( "command-line", commandline,m);
    }

    {
      // Get the time
      auto t = std::time(nullptr);
      auto tm = *std::localtime(&t);
      std::ostringstream oss;
      oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
      std::string time = oss.str();
      engine->Put( "time", time);
    }

    {
      // Iterate over all linked libraries.
      std::string exe(argv[0]);
      DistUtils::libData libNames;
      DistUtils::getAllLinkedLibraryData(&libNames);

      // Add all the libraries and the current exe to the json
      json exe_info = DistUtils::getLibInfo(exe.c_str(), 0);
      exe_info["libs"] = libNames.libs;
      engine->Put("exe-info", exe_info);
    }
    {
      // The configuration allows the user to specify additional files
      // that should be included in the output section. This allows for
      // versioning of things like input files. In this case, we load the entire
      // file into the output.
      // const json extra = getConfigurationJson();

      // if ( extra.find("input-files") != extra.end() ) {

      // json a = extra["input-files"];
      json ins = json::array();
      for (auto itt : inputFiles) {
        std::ifstream f(itt.get<std::string>());
        if (f.is_open()) {
          json r;
          std::stringstream sstr;
          sstr << f.rdbuf();
          r["file"] = sstr.str();
          f.close();
          r["info"] = DistUtils::getLibInfo(itt.get<std::string>(), 0);
          ins.push_back(r);
        }
        //}
        engine->Put( "input-files", ins);
      }
    }
    {
      // Throw the VnV configuration file into the output as well.
      engine->Put( "vnv-config", configFile);
    }

    return SUCCESS;
  }

  void writeTree(IOutputEngine* engine) {
    // TODO -- If the stage is looped. We can track changes to the file tree on
    // output and use that to track the outputs of the execution from the
    // current working directory.
  }
  void logTree() {
    // TODO Copy down the file tree in the current directory. This will be used
    // to track
    // which output files are generated during the Current looped injection
    // point.
  }
  static std::string provSchema() {
    return R"({
          "type":"object",
          "properties" : {
           },
           "additionalProperties" : false
           }
         )";
  }

  virtual ~provenanceRunner();
};

provenanceRunner::~provenanceRunner() {}

}  // namespace

/**
    Provenance Tracking
    ===================

    In this section, we detail the provenance tracking information for the
    sample executable. This includes a robust, fully detailed description of
    the software used to compile and run this code. This includes a detailed
    account of all shared libraries loaded in the application, as well as the
    time of execution, the command line used and the vnv configuration file.

    The configuration is:

    Current Working directory: :vnv:`Data.cwd.Value`

    Command Line: :vnv:`Data."command-line".Value`

    Time: :vnv:`Data.time.Value`

    The runtime config info was:

    .. vnv-jchart::
       :exe: Data."exe-info".Value
       :inp: Data."input-files".Value
       :conf: Data."vnv-config".Value
       :time: Data.time.Value
       :cmd: Data."command-line".Value
       :cwd: Data.cwd.Value

       {
         "Command Line" : $$cmd$$,
         "Working Directory" : $$cwd$$,
         "time" : $$time$$,
         "configuration File" : $$conf$$,
         "input Files" : $$inp$$,
         "exe info" : $$exe$$
       }



    TODO Represent this data in a nicer, less brute force way
 */

INJECTION_TEST_RS(VNVPACKAGENAME, provenance, provenanceRunner,
                  provenanceRunner::provSchema(), int* argc, char*** argv,
                  nlohmann::json config) {
  if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
    GetRef(c, "argc", int*);
    GetRef(f, "config", nlohmann::json);
    GetRef(v, "argv", char***);
    json confj = getConfigurationJson();
    auto it = confj.find("inputFiles");
    json inputFiles = (it == confj.end()) ? json::array() : it.value();
    return runner->getProvHistory(comm, engine, *c, *v, f, inputFiles);
  } else if (type == InjectionPointType::Begin) {
    runner->logTree();

  } else if (type == InjectionPointType::End) {
    runner->writeTree(engine);
  }
  return SUCCESS;
}

#endif
