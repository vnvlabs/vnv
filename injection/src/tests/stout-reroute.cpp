
/** @file provenance.cpp */

#ifndef _stdout_reroute_H
#define _stdout_reroute_H

#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include "vv-dist-utils.h"
#include "VnV-Interfaces.h"

using namespace VnV;

namespace StdRerouteTest {

class rerouteStdOut : public ITest {
    std::string stdoutName;
    std::string stderrName;

public:

   rerouteStdOut(TestConfig config) : ITest(config) {
        stderrName = std::tmpnam(nullptr);
        stdoutName = std::tmpnam(nullptr);
   }

   TestStatus initialize(IOutputEngine *engine, bool first) {
        if ( getConfigurationJson()["stdout"].get<bool>()) {
           if (first) std::cout << "ReRouting Stdout to " << stdoutName << std::endl;
           freopen(stdoutName.c_str(),"w",stdout);
        }
        if ( getConfigurationJson()["stderr"].get<bool>()) {
           if (first) std::cout << "ReRouting Stderr to " << stderrName << std::endl;
           freopen(stderrName.c_str(),"w",stderr);
        }
        return SUCCESS;
   }

   TestStatus finalize(IOutputEngine *engine, bool final) {
        if ( final ) flush(engine, true);
        if ( getConfigurationJson()["stdout"].get<bool>()) {
           freopen("/dev/stdout" , "w",stdout);
        }
        if ( getConfigurationJson()["stderr"].get<bool>()) {
           freopen("/dev/stderr","a",stderr);
        }
        return SUCCESS;
   }

   TestStatus flush(IOutputEngine *engine, bool fromFinal) {
       if ( !fromFinal ) finalize(engine, false);

       if ( getConfigurationJson()["stdout"].get<bool>()) {
           std::string ss = getFileToString(stdoutName);
           engine->Put("stdout", ss);
           std::cout << ss << std::endl;
        }
        if ( getConfigurationJson()["stderr"].get<bool>()) {
           std::string ss = getFileToString(stderrName);
           engine->Put("stderr", ss);
           std::cerr << ss << std::endl;
        }
        if ( !fromFinal ) initialize(engine, false);
        return SUCCESS;
   }

   std::string getFileToString(std::string filename) {
       std::ifstream f(filename);
       if (f.is_open())	{
          std::stringstream sstr;
          sstr << f.rdbuf();
          f.close();
          return sstr.str();
       }
       return "";
   }


  virtual TestStatus runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId, std::map<std::string, void*> &parameters) override {
       if (type == InjectionPointType::Begin ) return initialize(engine, true);
       else if (type == InjectionPointType::End ) return finalize(engine,true);
       else return flush(engine, false);
  }

  virtual ~rerouteStdOut() override;


};

rerouteStdOut::~rerouteStdOut() {}

ITest* maker(TestConfig config) { return new rerouteStdOut(config); }

json declare() {
    return R"({
            "name" : "reroute-stdout",
            "title" : "ReRoute stdout and stderr to the output engine.",
            "description" : "Moves the stdout and stderr streams to the output engine. This is a two stage test.  ",
            "expectedResult" : {"type" : "object" },
            "configuration" : {
               "type" : "object",
               "properties" : {
                  "stdout" : { "type" : "boolean"  },
                  "stderr" : { "type" : "boolean"  }
               }
            },
            "parameters" : {
            },
            "requiredParameters" : [],
            "io-variables" : {}
    })"_json;
}

}

#endif
