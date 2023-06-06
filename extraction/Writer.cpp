﻿#include <sys/file.h>
#include <sys/stat.h>

#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

#define LAST_RUN_TIME "__LAST_RUN_TIME__"
#define LAST_FILE_NAME "__LAST_FILE_NAME__"

using nlohmann::json;

void writeFortranModule(std::string package, std::string filename) {
  static std::string fortranModuleTemplate = R"(
! Autogenerated module for libvnv#### package. 

module libvnv####
    
    use iso_c_binding
    implicit none
    private
    public :: vnv_####_registration

    interface
        
  ! void functions maps to subroutines
    TYPE(C_PTR) function vnv_####_registration_c() bind(C, name="vnv_fort_####_x")
        use iso_c_binding
        implicit none
    end function

    end interface

contains ! Implementation of the functions. We just wrap the C function here.
    
    function vnv_####_registration() result(ptr)
        TYPE(C_PTR) :: ptr
        ptr = vnv_####_registration_c();
    end function
    
end module
)";

  // TODO COMPARE -- IF FILE IS SAME THEN DO NOT REWRITE SO IT DOESNT NEED
  // A REBUILD EVERY TIME.
  std::size_t has = -1;
  std::ifstream iffs(filename);

  std::string ho = "";

  std::ifstream efile(filename);
  if (efile.good()) {
    std::getline(efile, ho);
    efile.close();
  }

  std::string s = std::regex_replace(fortranModuleTemplate, std::regex("####"), package.c_str());
  std::size_t hs = std::hash<std::string>()(s);
  if (ho.size() <= 2 || std::to_string(hs).compare(ho.substr(2)) != 0) {
    std::ofstream ofs(filename);
    if (ofs.good()) {
      ofs << "! " << hs << "\n" << s;
    }
    ofs.close();
  }
}

class RegistrationWriter {
 public:
  std::map<std::string, std::ostringstream> oss_register;
  std::map<std::string, std::ostringstream> oss_declare;
  std::map<std::string, json> pjson;
  bool finalized = false;

  void createPackageOss(std::string packageName) {
    if (oss_register.find(packageName) == oss_register.end()) {
      std::ostringstream os_declare;
      os_declare << "//PACKAGENAME: " << packageName << "\n\n";
      os_declare << "#include \"VnV.h\" \n";
      oss_declare[packageName] = std::move(os_declare);

      std::ostringstream os_register;
      os_register << "INJECTION_REGISTRATION(" << packageName << "){\n";
      oss_register[packageName] = std::move(os_register);
      pjson[packageName] = json::object();
    }
  }

  void finalizePackage(std::string packageName) {
    auto it = oss_register.find(packageName);
    if (it != oss_register.end()) {
      it->second << "\tREGISTER_FULL_JSON(" << packageName << ", getFullRegistrationJson_" << it->first << ");\n";

      it->second << "}\n\n";

      oss_declare[packageName] << "const char* getFullRegistrationJson_" << it->first << "(){"
                               << "\n"
                               << "\t return " << VnV::StringUtils::escapeQuotes(pjson[it->first].dump(), true)
                               << ";}\n\n"
                               << it->second.str() << "\n\n"
                               << "FORTRAN_INJECTION_REGISTRATION(" << packageName << ")\n\n";
    }
  }

  void finalizeAllPackages() {
    if (!finalized) {
      for (auto& it : oss_register) {
        finalizePackage(it.first);
      }
    }
    finalized = true;
  }

  json getExecutableDocumentations(std::string packageName) {
    auto it = pjson.find(packageName);
    if (it != pjson.end()) {
      if (it->second.contains("Executables")) {
        return (it->second)["Executables"];
      }
    }
    return json::object();
  }

  std::string printPackage(std::string packageName) {
    if (!packageName.empty()) {
      auto it = oss_declare.find(packageName);
      if (it != oss_declare.end()) {
        std::string s = it->second.str();
        return s;
      } else {
        // This means the package includes VnV.h, but does not have any VnV
        // INJECTION CALLS. So, we should return a empty configuration.
        createPackageOss(packageName);
        finalizePackage(packageName);
        return oss_declare[packageName].str();
      }
    }
    // Otherwise, create a json object containing all of them.
    json j = json::object();
    for (auto& it : oss_declare) {
      j[it.first] = it.second.str();
    }
    return j.dump();
  }

  void registerHelper(json& j, std::string key, std::string reg, std::string pname) {
    if (j.contains(key)) {
      for (auto it : j[key].items()) {
        std::string packageName = it.value()["packageName"];
        if (pname.empty() || packageName == pname) {
          std::string name = it.value()["name"];
          createPackageOss(packageName);

          // Save the json, declare the engine, register the engine.
          it.value().erase("name");
          it.value().erase("packageName");
          VnV::JsonUtilities::getOrCreate(pjson[packageName], key)[name] = it.value();

          oss_declare[packageName] << "DECLARE" << reg << "(" << packageName << "," << name << ")\n";
          oss_register[packageName] << "\tREGISTER" << reg << "(" << packageName << "," << name << ");\n";
        }
      }
    }
  }

  RegistrationWriter(json& j, std::string packageName = "") {
    registerHelper(j, "Tests", "TEST", packageName);
    registerHelper(j, "Iterators", "ITERATOR", packageName);
    registerHelper(j, "Plugs", "PLUG", packageName);
    registerHelper(j, "Engines", "ENGINE", packageName);
    registerHelper(j, "EngineReaders", "ENGINEREADER", packageName);
    registerHelper(j, "UnitTests", "UNITTEST", packageName);
    registerHelper(j, "Actions", "ACTION", packageName);
    registerHelper(j, "Serializers", "SERIALIZER", packageName);
    registerHelper(j, "Transforms", "TRANSFORM", packageName);
    registerHelper(j, "Reducers", "REDUCER", packageName);
    registerHelper(j, "ScriptGenerators", "SCRIPTGENERATOR", packageName);
    registerHelper(j, "Validators", "VALIDATOR", packageName);
    registerHelper(j, "JobCreators", "JOBCREATOR", packageName);
    registerHelper(j, "Schedulers", "SCHEDULER", packageName);
    registerHelper(j, "Samplers", "SAMPLER", packageName);
    registerHelper(j, "Walkers", "WALKER", packageName);
    registerHelper(j, "DataTypes", "DATATYPE", packageName);

    if (j.contains("CodeBlocks")) {
      for (auto& it : j["CodeBlocks"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();
        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          createPackageOss(pname);
          VnV::JsonUtilities::getOrCreate(pjson[pname], "CodeBlocks")[n] = it.value()["code"];
        }
      }
    }

    if (j.contains("LogLevels")) {
      for (auto it : j["LogLevels"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();

        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          std::string c = it.value()["color"].get<std::string>();
          createPackageOss(pname);
          oss_register[pname] << "\tREGISTERLOGLEVEL(" << pname << " , " << n << "," << c << ");\n";
          VnV::JsonUtilities::getOrCreate(pjson[pname], "LogLevels")[n] = it.value();
        }
      }
    }

    if (j.contains("Files")) {
      for (auto it : j["Files"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();
        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();

          createPackageOss(pname);

          VnV::JsonUtilities::getOrCreate(pjson[pname], "Files")[n] = it.value();
        }
      }
    }

    if (j.contains("SubPackages")) {
      for (auto it : j["SubPackages"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();
        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          createPackageOss(pname);
          oss_declare[pname] << "DECLARESUBPACKAGE(" << n << ")\n";
          oss_register[pname] << "\tREGISTERSUBPACKAGE(" << n << ");\n";
          VnV::JsonUtilities::getOrCreate(pjson[pname], "SubPackages")[n] = it.value();
        }
      }
    }

    // Catch the options. They are a little different from above (no name
    // parameter).
    if (j.contains("Options")) {
      for (auto it : j["Options"].items()) {
        std::string pname = it.key();
        if (packageName.empty() || pname == packageName) {
          createPackageOss(pname);

          if (it.value().contains("schema")) {
            oss_register[pname] << "\tREGISTERRAWOPTIONS(" << pname << ","
                                << VnV::StringUtils::escapeQuotes(it.value()["schema"].dump(), true) << ")\n";
          } else {
            oss_declare[pname] << "DECLAREOPTIONS(" << pname << ")\n";
            oss_register[pname] << "\tREGISTEROPTIONS(" << pname << ")\n";
          }
          pjson[pname]["Options"] = it.value();
        }
        if (packageName == pname) break;
      }
    }

    // Add the intro and the conclusion.
    std::vector<std::string> cc = {"Introduction", "Conclusion", "Executables"};
    for (std::string i : cc) {
      if (j.contains(i)) {
        for (auto it : j[i].items()) {
          std::string pname = it.key();
          if (packageName.empty() || pname.compare(packageName) == 0) {
            createPackageOss(packageName);
            pjson[pname][i] = it.value();
          }
        }
      }
    }

    // Catch the injection points

    if (j.contains("InjectionPoints")) {
      for (auto it : j["InjectionPoints"].items()) {
        std::string pname = it.value()["packageName"];
        if (packageName.empty() || pname == packageName) {
          std::string name = it.value()["name"].get<std::string>();

          json& params = it.value()["parameters"];

          createPackageOss(pname);

          VnV::JsonUtilities::getOrCreate(pjson[pname], "InjectionPoints")[name] = it.value();

          bool iterator = it.value().value("/iterator"_json_pointer, false);
          bool plug = it.value().value("/plug"_json_pointer, false);
          std::string escaped = VnV::StringUtils::escapeQuotes(params.dump(), true);
          std::string t = iterator ? "Iterator" : plug ? "Plug" : "Point";

          oss_register[pname] << "\tRegister_Injection_" << t << "(\"" << pname << "\",\"" << name << "\"," << escaped
                              << ");\n";
        }
      }
    }
    finalizeAllPackages();
  }
};

void writeFile(json& cacheInfo, std::string outputFileName, std::string regFileName, std::string targetFileName,
               std::string packageName, std::string strip, std::string fortran) {
  // First we load the existing file in that directory.
  bool cacheChanged = true;

  // Pull everything into types, not files.
  json finalJson = json::object();
  for (auto it : cacheInfo["data"].items()) {
    for (std::string type :
         {"InjectionPoints", "SubPackages",  "LogLevels",        "Files",       "Tests",        "Iterators",
          "Plugs",           "Engines",      "EngineReaders",    "Comms",       "Reducers",     "Samplers",
          "Walkers",         "DataTypes",    "Serializers",      "Transforms",  "UnitTests",    "Actions",
          "Options",         "Introduction", "Conclusion",       "Executables", "Communicator", "Schedulers",
          "Validators",      "JobCreators",  "ScriptGenerators", "CodeBlocks"}) {
      json& to = VnV::JsonUtilities::getOrCreate(finalJson, type);
      for (auto it : VnV::JsonUtilities::getOrCreate(it.value(), type).items()) {
        to[it.key()] = it.value();
      }
    }
  }

  // Generate the registration code using the given json.
  RegistrationWriter r(finalJson, packageName);

  std::ostringstream oss;
  oss << "/// This file was automatically generated using the VnV-Matcher "
         "executable. \n"
         "/// The matcher allows for automatic registration of all VnV "
         "plugins and injection \n"
         "/// points. Building the matcher requires Clang. If Clang is not "
         "available on this machine,\n"
         "/// Registration code should be written manually. \n"
         "/// \n\n";

  oss << r.printPackage(packageName);
  std::string outputText = oss.str();

  std::size_t h = std::hash<std::string>()(oss.str());

  // First line of a file is a hash representing its content.
  std::ifstream efile(outputFileName);
  std::string ho = "";
  if (efile.good()) {
    std::getline(efile, ho);
    efile.close();
  }

  // If the hash changed then rewrite the file -- this avoid recompilation
  // of the file every time we run because we only actually touch the file
  // if it changes.
  std::string nhash = "//" + std::to_string(h);
  if (ho.compare(nhash) != 0) {
    std::ofstream ofile(outputFileName);
    if (ofile.good()) {
      ofile << nhash << "\n" << outputText;
      ofile.close();
    } else {
      std::cout << "---->Error: Bad output file name" << std::endl;
    }
  }

  // Update the fortran module file.
  if (fortran.size() > 0) {
    writeFortranModule(packageName, fortran);
  }

  // Updating the registration file
  json jc = r.getExecutableDocumentations(packageName);


  if ( !regFileName.empty() && !targetFileName.empty()) {

    // Get a lock file for the registration file.
    std::string lockFile = "." + regFileName + ".lock";
    int fd = open(lockFile.c_str(), O_WRONLY | O_CREAT);
    flock(fd, LOCK_EX);

    std::ifstream ifv(regFileName);
    json j;
    if (ifv.good()) {
      
      try {
        j = json::parse(ifv);
        assert(j.is_object());
      } catch (...) {
        std::cout << "Invalid registration file. Please fix/delete it and try again" << std::endl;
      }

    } else {
      j = R"({"reports":{},"executables": {}, "plugins" : {}, "libraries" : {} })"_json;
    }

    if (!j.contains("executables")) {
      j["executables"] = json::object();
    }
    if (!j.contains("plugins")) {
      j["plugins"] = json::object();
    }
    if (!j.contains("libraries")) {
      j["libraries"] = json::object();
    }

    if ( jc.size() > 0 ) {
      
      json jv = json::object();
      jv["filename"] = targetFileName;
      jv["description"] = jc.value("description", "");
      jv["packageName"] = packageName;
      if (jc.contains("configuration")) {
        jv["defaults"] = jc["configuration"];
      }

      std::string n = jc.value("title", packageName);
      if (n.empty()) n = packageName;

      std::string ty = jc.value("lib", "executables");

      // hmmmmmm
      assert(ty.compare("executables") == 0 || ty.compare("libraries") == 0 || ty.compare("plugins") == 0);

      j[ty][n] = jv;  ////// TODO

      //Register it in the users global env place.
      const char *homedir = getenv("HOME");
      std::string homed(homedir);
      homed += "/.vnv";
      std::string lockFile1 = homed + ".lock";
      
      int fdd = open(lockFile1.c_str(), O_WRONLY | O_CREAT);
      flock(fdd, LOCK_EX);

      std::set<std::string> files;
      files.insert(regFileName);

      std::ifstream globalF(homed);
      if (globalF.good()) {
        std::string nextline;
        while (std::getline(globalF, nextline)) {
          files.insert(nextline); 
        }
      }
      globalF.close();

      std::ofstream globalO(homed);
      for (auto &it : files) {
        globalO << it << "\n";
      }
      globalO.close();

      flock(fdd, LOCK_UN);

    }

    //Write and unlock
    std::ofstream rfile(regFileName);
    rfile << j.dump(3);
    rfile.close();
    flock(fd, LOCK_UN);
  }
}

void removeNonModified(std::set<std::string>& files, json& cacheFiles, std::map<std::string, bool>& fModMap,
                       std::string mess) {
  std::set<std::string> nonModFiles;
  for (auto& it : files) {  // all files to be compiled. (strings)
    if (cacheFiles.contains(it)) {
      bool mods = false;
      for (auto f : cacheFiles[it].items()) {
        // list of ids included in this file.
        std::string id = f.value().get<std::string>();
        if (fModMap[id]) {
          mods = true;
          break;
        }
      }
      if (!mods) {
        nonModFiles.insert(it);
      }
    }
  }

  for (auto it : nonModFiles) {
    files.erase(it);
  }

  if (files.size() > 0) {
    std::ostringstream os;
    os << "----> The following " << mess << " files will be reparsed:" << std::endl;

    for (auto it : files) {
      os << "--------> " << it << std::endl;
    }
    std::cout << os.str() << std::endl;
    ;
  }
}

// Returns a list of files that have changed since the cache last ran.
void checkCache(json& cacheInfo, std::set<std::string>& files, std::set<std::string>& ffiles) {
  json& cacheMap = VnV::JsonUtilities::getOrCreate(cacheInfo, "map");
  json& cacheFiles = VnV::JsonUtilities::getOrCreate(cacheInfo, "files");

  bool hasCache = (cacheInfo.contains(LAST_RUN_TIME));

  std::set<std::string> nonModFiles;

  if (hasCache) {
    std::string lastRunTime = cacheInfo[LAST_RUN_TIME].get<std::string>();

    std::map<std::string, bool> fModMap;

    bool write = false;
    std::ostringstream oss;
    oss << "----> The following files have changed since we last ran:" << std::endl;

    for (auto it : cacheMap.items()) {
      auto r = (VnV::TimeUtils::timeForFile(it.value().get<std::string>()) > lastRunTime);
      fModMap[it.key()] = r;
      if (r) {
        write = true;
        oss << "--------> " << it.value().get<std::string>() << std::endl;
      }
    }
    if (write) {
      std::cout << oss.str() << std::endl;
    }
    removeNonModified(files, cacheFiles, fModMap, "C/C++");
    removeNonModified(ffiles, cacheFiles, fModMap, "Fortran");
  }
}
