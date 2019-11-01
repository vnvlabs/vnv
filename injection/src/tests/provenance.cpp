/** @file provenance.cpp */

#ifndef _provenance_H
#define _provenance_H


#include <link.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "vv-utils.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

#include "VnV-Interfaces.h"
#include "vv-logging.h"
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

json getLibInfo(std::string filepath, unsigned long add) {

    struct stat result;
    stat(filepath.c_str(),&result);
    json libJson;
    libJson["name"] = filepath;
    libJson["add"] = add;
    libJson["hash"] = hashfile(filepath);
    libJson["st_dev"] = result.st_dev;
    libJson["st_gid"] = result.st_gid;
    libJson["st_ino"] = result.st_ino;
    libJson["st_uid"] = result.st_uid;
    libJson["st_atim_sec"] = result.st_atim.tv_sec;
    libJson["st_atim_nsec"] = result.st_atim.tv_nsec;
    libJson["st_ctim_sec"] = result.st_ctim.tv_sec;
    libJson["st_ctim_nsec"] = result.st_ctim.tv_nsec;
    libJson["st_mtim_sec"] = result.st_mtim.tv_sec;
    libJson["st_mtim_nsec"] = result.st_mtim.tv_nsec;
    libJson["st_mode"] = result.st_mode;
    libJson["st_rdev"] = result.st_rdev;
    libJson["st_size"] = result.st_size;
    libJson["st_nlink"] = result.st_nlink;
    libJson["st_blocks"] = result.st_blocks;
    libJson["st_blksize"] = result.st_blksize;
    return libJson;
}

static int callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {

  std::string name(info->dlpi_name);
  unsigned long add(info->dlpi_addr);
  libData* x = (libData*) data;
  x->libs.push_back(getLibInfo(name,add));
  return 0;
}

class provenance : public ITest {
 public:
  TestStatus runTest(IOutputEngine* engine, int argc, char** argv,
                     std::string configFile) {

    {
        std::string currentWorkingDirectory(get_current_dir_name());
        engine->Put("cwd", currentWorkingDirectory);
    }

    {
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
        std::string exe(argv[0]);
        libData libNames;
        dl_iterate_phdr(callback, &libNames);
        json exe_info = getLibInfo(exe.c_str(),0);
        exe_info["libs"] = libNames.libs;
        std::string exe_i = exe_info.dump();
        engine->Put("exe-info",exe_i);
    }
    {

       json extra = m_config.getAdditionalProperties();
       if ( extra.find("input-files") != extra.end() ) {

         json a = extra["input-files"];
         std::vector<json> ins;
         for ( auto itt: a ) {
               std::cout << a;
               std::ifstream f(itt.get<std::string>());
               if (f.is_open())	{
                   json r;
                   std::stringstream sstr;
                   sstr << f.rdbuf();
                   r["file"] = sstr.str();
                   f.close();
                   r["info"] = getLibInfo(itt.get<std::string>(),0);
                   ins.push_back(r);
               }
         }
         json x = ins;
         std::string xx = x.dump();
         engine->Put("input-files",xx);

      }
   }
   {
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

  static void DeclareIO(IOutputEngine* /*engine*/) {}

  void init() override {
    m_parameters.insert(std::make_pair("argc", "int*"));
    m_parameters.insert(std::make_pair("argv", "char***"));
    m_parameters.insert(std::make_pair("config", "std::string"));
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

extern "C" {
ITest* provenance_maker() { return new provenance(); }

void provenance_DeclareIO(IOutputEngine* engine) {
  provenance::DeclareIO(engine);
}
};

class provenance_proxy {
 public:
  provenance_proxy() {
    VnV_registerTest("provenance", provenance_maker, provenance_DeclareIO);
  }
};

provenance_proxy prov_proxy;

#endif
