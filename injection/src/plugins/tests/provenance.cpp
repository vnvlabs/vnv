/** @file provenance.cpp */

#ifndef _provenance_H
#define _provenance_H

#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>

#include "base/DistUtils.h"
#include "interfaces/ITest.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

namespace {

/**
 * This test is a test that checks the provenance of the file.
 */
class provenanceRunner {
 public:

    provenanceRunner() {}

    TestStatus getProvHistory(VnV_Comm comm, IOutputEngine* engine, int argc, char** argv,
                     json configFile, json& inputFiles) {

    {
        // Add the current working directory
        std::string currentWorkingDirectory(DistUtils::getCurrentDirectory());
        engine->Put(comm,"cwd", currentWorkingDirectory);
    }

    {
        // Add the command line
        std::string commandline = argv[0];
        for (int i = 1; i < argc; i++) {
          std::string v(argv[i]);
          commandline += " " + std::string(argv[i]);
        }
        engine->Put(comm,"command-line", commandline);
    }

    {
        // Get the time
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
        std::string time = oss.str();
        engine->Put(comm,"time", time);
     }

     {
        // Iterate over all linked libraries.
        std::string exe(argv[0]);
        DistUtils::libData libNames;
        DistUtils::getAllLinkedLibraryData(&libNames);

        //Add all the libraries and the current exe to the json
        json exe_info = DistUtils::getLibInfo(exe.c_str(),0);
        exe_info["libs"] = libNames.libs;
        engine->Put(comm,"exe-info",exe_info);
    }
    {
       // The configuration allows the user to specify additional files
       // that should be included in the output section. This allows for versioning
       // of things like input files. In this case, we load the entire file into the
       // output.
       //const json extra = getConfigurationJson();

       //if ( extra.find("input-files") != extra.end() ) {

        // json a = extra["input-files"];
         json ins = json::array();
         for ( auto itt: inputFiles ) {
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
         //}
           engine->Put(comm,"input-files",ins);
      }
   }
   {
     //Throw the VnV configuration file into the output as well.
     engine->Put(comm,"vnv-config", configFile);
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

   virtual ~provenanceRunner();
};

provenanceRunner::~provenanceRunner() {}

}

INJECTION_TEST_R(provenance, provenanceRunner, int* argc, char*** argv, json config) {
   if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
       GetRef(c,"argc",int*);
       GetRef(f,"config",json);
       GetRef(v,"argv",char***);
       json confj = getConfigurationJson();
       auto it = confj.find("inputFiles");
       json inputFiles = (it==confj.end()) ? json::array() : it.value();
       return runner->getProvHistory(comm,engine, *c, *v, f,inputFiles);
    }
    else if ( type == InjectionPointType::Begin) {
       runner->logTree();

     }
    else if (type == InjectionPointType::End){
       runner->writeTree(engine);
    }
    return SUCCESS;
}

#endif
