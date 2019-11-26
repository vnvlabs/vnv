/** @file provenance.cpp */

#ifndef _provenance_H
#define _provenance_H

#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip>
#include "vv-dist-utils.h"
#include "VnV-Interfaces.h"

using namespace VnV;

struct libInfo {
    std::string name;
};

struct libData
{
public:
    std::vector<json> libs;
    libData() {}
};

static int callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {

  std::string name(info->dlpi_name);
  unsigned long add(info->dlpi_addr);
  libData* x = static_cast<libData*>(data);
  x->libs.push_back(DistUtils::getLibInfo(name,add));
  return 0;
}

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
        libData libNames;
        DistUtils::iterateLinkedLibraries(callback, &libNames);

        //Add all the libraries and the current exe to the json
        json exe_info = DistUtils::getLibInfo(exe.c_str(),0);
        exe_info["libs"] = libNames.libs;
        std::string exe_i = exe_info.dump();
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
     conf["name"] = configFile;
     std::ifstream ff(configFile);
     if (ff.is_open()) {
        std::stringstream ss;
        ss << ff.rdbuf();
        conf["file"] = ss.str();
     }
     std::string confx = conf.dump();
     engine->Put("vnv-config", confx);
    }

    return SUCCESS;
  }

    virtual TestStatus runTest(IOutputEngine* engine, int stage,
                             NTV& parameters) override {
    
    VnV_Debug("RUNNING PROVENANCE TEST");	  
    char**** v = carefull_cast<char***>(stage, "argv", parameters);
    int** c = carefull_cast<int*>(stage, "argc", parameters);
    std::string* f = carefull_cast<std::string>(stage, "config", parameters);
    return runTest(engine, **c, **v, *f);
  }

  virtual ~provenance() override;
};

provenance::~provenance() {}

ITest* provenance_maker(TestConfig config) { return new provenance(config); }

json provenance_Declare() {
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

class provenance_proxy {
 public:
  provenance_proxy() {
    VnV_registerTest("provenance", provenance_maker, provenance_Declare);
  }
};

provenance_proxy prov_proxy;

#endif
