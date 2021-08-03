﻿#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "base/Utilities.h"
#include "json-schema.hpp"

#define LAST_RUN_TIME "__LAST_RUN_TIME__"
#define LAST_FILE_NAME "__LAST_FILE_NAME__"

using nlohmann::json;

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
      os_declare << "#include \"version/version.h\"\n";
      os_declare << "\n\nVNVVERSIONINFOCALLBACK(" << packageName << ")\n\n";
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
      it->second << "\tREGISTER_FULL_JSON(" << packageName
                 << ", getFullRegistrationJson_" << it->first << ");\n";
      it->second << "};\n\n";
      oss_declare[packageName]
          << "const char* getFullRegistrationJson_" << it->first << "(){"
          << "\n"
          << "\t return "
          << VnV::StringUtils::escapeQuotes(pjson[it->first].dump(), true)
          << ";}\n\n"
          << it->second.str() << "\n\n";
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

  std::string printPackage(std::string packageName) {
    if (!packageName.empty()) {
      auto it = oss_declare.find(packageName);
      if (it != oss_declare.end()) {
        return it->second.str();
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

  void registerHelper(json& j, std::string key, std::string reg,
                      std::string pname) {
    if (j.contains(key)) {
      for (auto it : j[key].items()) {
        std::string packageName = it.value()["packageName"];
        if (pname.empty() || packageName == pname) {
          std::string name = it.value()["name"];
          createPackageOss(packageName);

          // Save the json, declare the engine, register the engine.
          it.value().erase("name");
          it.value().erase("packageName");
          VnV::JsonUtilities::getOrCreate(pjson[packageName], key)[name] =
              it.value();

          oss_declare[packageName] << "DECLARE" << reg << "(" << packageName
                                   << "," << name << ")\n";
          oss_register[packageName] << "\tREGISTER" << reg << "(" << packageName
                                    << "," << name << ");\n";
        }
      }
    }
  }

  RegistrationWriter(json& j, std::string packageName = "") {
    registerHelper(j, "Tests", "TEST", packageName);
    registerHelper(j, "Iterators", "ITERATOR", packageName);
    registerHelper(j, "Plugs", "PLUG", packageName);
    registerHelper(j, "Engines", "ENGINE", packageName);
    registerHelper(j, "UnitTests", "UNITTEST", packageName);
    registerHelper(j, "Actions", "ACTION", packageName);
    registerHelper(j, "Serializers", "SERIALIZER", packageName);
    registerHelper(j, "Transforms", "TRANSFORM", packageName);
    registerHelper(j, "Reducers", "REDUCER", packageName);
    registerHelper(j, "Samplers", "SAMPLER", packageName);
    registerHelper(j, "DataTypes", "DATATYPE", packageName);
    if (j.contains("LogLevels")) {
      for (auto it : j["LogLevels"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();

        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          std::string c = it.value()["color"].get<std::string>();
          createPackageOss(pname);
          oss_register[pname] << "\tREGISTERLOGLEVEL(" << pname << " , " << n
                              << "," << c << ");\n";
          VnV::JsonUtilities::getOrCreate(pjson[pname], "LogLevels")[n] =
              it.value();
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
          oss_register[pname] << "\tREGISTERSUBPACKAGE(" << pname << "," << n
                              << ");\n";
          VnV::JsonUtilities::getOrCreate(pjson[pname], "SubPackages")[n] =
              it.value();
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
          oss_declare[pname] << "DECLAREOPTIONS(" << pname << ")\n";
          oss_register[pname] << "\tREGISTEROPTIONS(" << pname << ")\n";
          pjson[pname]["Options"] = it.value();
        }
        if (packageName == pname) break;
      }
    }

    // Add the intro and the conclusion.
    std::vector<std::string> cc = {"Introduction", "Conclusion", "Package"};
    for (std::string i : cc) {
      if (j.contains(i)) {
        for (auto it : j[i].items()) {
          std::string pname = it.key();
          if (packageName.empty() || pname == packageName) {
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

          json& params =
              it.value()["parameters"]
                        [0];  // TODO TEMPLATE ARGUEMENTS NOT SUPPORTED ?

          pjson[pname]["InjectionPoints"][name] = it.value();
          createPackageOss(pname);

          bool iterator = it.value().value("/iterator"_json_pointer, false);
          bool plug = it.value().value("/plug"_json_pointer, false);
          std::string escaped =
              VnV::StringUtils::escapeQuotes(params.dump(), true);
          std::string t = iterator ? "Iterator" : plug ? "Plug" : "Point";

          oss_register[pname] << "\tRegister_Injection_" << t << "(\"" << pname
                              << "\",\"" << name << "\"," << escaped << ");\n";
        }
      }
    }
    finalizeAllPackages();
  }
};

void writeFile(json& cacheInfo, std::string outputFileName,
               std::string cacheFile, std::string packageName, bool force) {
  std::size_t h = std::hash<std::string>()(cacheInfo.dump());
  std::string hashStr = "///" + packageName + ":" + std::to_string(h);

  // First we load the existing file in that directory.
  if (!force) {
    std::ifstream efile(outputFileName);
    if (efile.good()) {
      // If it exists, we read the first line and get the cacheData hash.
      std::string sLine;
      std::getline(efile, sLine);
      if (sLine.compare(hashStr) == 0) {
        // The cache hasnt changed, so this file definetly hasn't changed.
        efile.close();
        return;
      }
    }
    efile.close();
  }
  
  // Pull everything into types, not files.
  json finalJson = json::object();
  for (auto it : cacheInfo["data"].items()) {
    for (std::string type :
         {"InjectionPoints", "SubPackages", "LogLevels", "Tests", "Iterators",
          "Plugs", "Engines", "Comms", "Reducers", "Samplers", "DataTypes", "Serializers",
          "Transforms", "UnitTests", "Actions", "Options", "Introduction",
          "Conclusion", "Package", "Communicator"}) {
      json& to = VnV::JsonUtilities::getOrCreate(finalJson, type);
      for (auto it :
           VnV::JsonUtilities::getOrCreate(it.value(), type).items()) {
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

  // If the file exists, load it and compare it to the new string. This is
  // TODO -- This is a stop-gap measure to avoid recompilation when the file
  // doesn't actually change.

  // Write the file.
  std::ofstream ofile(outputFileName);
  ofile << hashStr << "\n";
  ofile << oss.str();
  ofile.close();

  if (cacheFile.empty()) {
    return;
  }
  if (cacheInfo.contains("CACHEHASH") &&
      cacheInfo["CACHEHASH"].get<std::size_t>() == h) {
    return;  // Cache is the same, no need to rewrite it.
  }

  std::ofstream cacheStream(cacheFile);
  if (!cacheStream.good()) {
    return;
  }

  // Do some cleaning of the data for size.
  json& d = cacheInfo["data"];
  for (auto it = d.begin(); it != d.end();) {
    if (it.value().is_object() && it.value().size() == 0) {
      it = d.erase(it);
    } else {
      ++it;
    }
  }

  json a = json::array();
  for (auto& it : r.oss_declare) {
    a.push_back(it.first);
  }
  cacheInfo["Packages"] = a;

  // get a list of all includes used in the different files.
  std::hash<std::string> hasher;
  std::set<std::string> used;
  for (auto it : cacheInfo["files"].items()) {
    for (auto itt : it.value().items()) {
      used.insert(itt.value().get<std::string>());
    }
    used.insert(std::to_string(hasher(it.key())));
  }

  // remove any files from the map that are not used.
  json& m = cacheInfo["map"];
  for (auto it = m.begin(); it != m.end();) {
    if ((used.end() == used.find(it.key()))) {
      it = m.erase(it);
    } else {
      ++it;
    }
  }

  // Write the cache.
  time_t now;
  time(&now);
  cacheInfo[LAST_RUN_TIME] = VnV::TimeUtils::timeToISOString(&now);
  cacheInfo["CACHEHASH"] = h;
  cacheStream << cacheInfo.dump();
  cacheStream.close();
}

// Returns a list of files that have changed since the cache last ran.
std::set<std::string> checkCache(json& cacheInfo,
                                 std::set<std::string>& files) {
  json& cacheMap = VnV::JsonUtilities::getOrCreate(cacheInfo, "map");
  json& cacheFiles = VnV::JsonUtilities::getOrCreate(cacheInfo, "files");

  bool hasCache = (cacheInfo.contains(LAST_RUN_TIME));

  std::set<std::string> modFiles;
  if (hasCache) {
    std::string lastRunTime = cacheInfo[LAST_RUN_TIME].get<std::string>();

    std::map<std::string, bool> fModMap;

    for (auto it : cacheMap.items()) {
      fModMap[it.key()] = (VnV::TimeUtils::timeForFile(
                               it.value().get<std::string>()) > lastRunTime);
    }

    for (auto& it : files) {  // all files to be compiled. (strings)
      if (cacheFiles.contains(it)) {
        for (auto f : cacheFiles[it].items()) {
          // list of ids included in this file.
          std::string id = f.value().get<std::string>();
          if (fModMap[id]) {
            modFiles.insert(it);
            break;
          }
        }
      } else {
        modFiles.insert(it);  // New File not prev in cache.
      }
    }
    return modFiles;
  } else {
    return files;
  }
}
