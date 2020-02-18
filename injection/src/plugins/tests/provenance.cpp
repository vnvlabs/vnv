/** @file provenance.cpp */

#ifndef _provenance_H
#define _provenance_H

#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>

#include "base/DistUtils.h"
#include "interfaces/itest.h"
#include "c-interfaces/logging-interface.h"

using namespace VnV;

namespace ProvenanceTest {

class provenance : public ITest {
 public:

    provenance(TestConfig config) : ITest(config) {

    }

    TestStatus runTest(IOutputEngine* engine, int argc, char** argv,
                     std::string configFile) {

    {
        // Add the current working directory
        std::string currentWorkingDirectory(DistUtils::getCurrentDirectory());
        engine->Put("cwd", currentWorkingDirectory);
    }

    {
        // Add the command line
        std::string commandline = argv[0];
        for (int i = 1; i < argc; i++) {
          std::string v(argv[i]);
          commandline += " " + std::string(argv[i]);
        }
        engine->Put("command-line", commandline);
    }

    {
        // Get the time
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
        std::string time = oss.str();
        engine->Put("time", time);
     }

     {
        // Iterate over all linked libraries.
        std::string exe(argv[0]);
        DistUtils::libData libNames;
        DistUtils::getAllLinkedLibraryData(&libNames);

        //Add all the libraries and the current exe to the json
        json exe_info = DistUtils::getLibInfo(exe.c_str(),0);
        exe_info["libs"] = libNames.libs;
        std::string exe_i = exe_info.dump(2);
        engine->Put("exe-info",exe_i);
    }
    {
       // The configuration allows the user to specify additional files
       // that should be included in the output section. This allows for versioning
       // of things like input files. In this case, we load the entire file into the
       // output.
       const json extra = getConfigurationJson();

       if ( extra.find("input-files") != extra.end() ) {

         json a = extra["input-files"];
         std::vector<json> ins;
         for ( auto itt: a ) {
               std::ifstream f(itt.get<std::string>());
               if (f.is_open())	{
                   json r;
                   std::stringstream sstr;
                   sstr << f.rdbuf();
                   r["file"] = sstr.str();
                   f.close();
                   r["info"] = DistUtils::getLibInfo(itt.get<std::string>(),0);
                   ins.push_back(r);
               }
         }
         json x = ins;
         std::string xx = x.dump();
         engine->Put("input-files",xx);

      }
   }
   {
     //Throw the VnV configuration file into the output as well.
     json conf;
     conf["name"] = DistUtils::getAbsolutePath(configFile);

     std::ifstream ff(configFile);
     conf["file"] = json::parse(ff);

     std::string confx = conf.dump(2);
     engine->Put("vnv-config", confx);
    }

    return SUCCESS;
  }

   void writeTree(IOutputEngine *engine) {
       // TODO -- If the stage is looped. We can track changes to the file tree on output and
       // use that to track the outputs of the execution from the current working directory.
   }
   void logTree() {
       //TODO Copy down the file tree in the current directory. This will be used to track
       // which output files are generated during the Current looped injection point.
   }

   virtual TestStatus runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId,
                             std::map<std::string, void*>& parameters) override {
      TestStatus r = SUCCESS;
       
      if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
            
            VnV_Debug("RUNNING PROVENANCE TEST");
            char*** v = *static_cast<char****>(parameters["argv"]);
            int* c = *static_cast<int**>(parameters["argc"]);
            std::string f = *static_cast<std::string*>(parameters["config"]);
            r = runTest(engine, *c, *v, f);
     }
     if ( type == InjectionPointType::Begin) {
        logTree();
     }
     else if (type == InjectionPointType::End){
        writeTree(engine);
     }
     return r;
  }

  virtual ~provenance() override;
};

provenance::~provenance() {}

ITest* maker(TestConfig config) { return new provenance(config); }

json declare() {
    return R"({
            "name" : "provenance",
            "title" : "Provenance Tracking.",
            "description" : "This test tracks provenance for the executable, including info about all linked libraries",
            "expectedResult" : {"type" : "object" },
            "configuration" : {
               "type" : "object",
               "properties" : {
                  "input-files" : { "type" : "array" , "items" : {"type" : "string"} }
               }
            },
            "parameters" : {
               "argc" : "int*",
               "argv" : "char***",
               "config" : "std::string"
            },
            "requiredParameters" : ["argc","argv","config"],
            "io-variables" : {}
    })"_json;
}
}

#endif
