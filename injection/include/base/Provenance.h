#ifndef PROVANENCE_HEADER
#define PROVANENCE_HEADER
#include <iostream>
#include <string>
#include <vector>

#include "base/DistUtils.h"
#include "json-schema.hpp"
using nlohmann::json;

namespace VnV {

class ProvFile {
 public:
  std::string filename = "<unset>";
  std::string reader = "";
  std::string text = "";
  std::string package = "root";
  std::string name = "<none>";
  long comm = -1;

  VnV::DistUtils::libInfo info;

  ProvFile();
  ProvFile(std::string filename, std::string reader, std::string text = "");
  ProvFile(VnV::DistUtils::libInfo lb, std::string reader);
  ProvFile(const json& j);

  json toJson() const;

  bool modified() const;

  json getDataChildren() {
    json o = json::object();
    o["text"] = name;
    o["icon"] = "feather icon-file";
    o["children"] = json::array();
    
    if (text.size() > 0 ) {
      json j = json::object();
      j["text"] = "Text: " + text;
      j["icon"] = "feather icon-minus";
      o["children"].push_back(j);
    }

    {
      json j = json::object();
      j["text"] = "Package: " + package;
      j["icon"] = "feather icon-minus";
      o["children"].push_back(j);
    }
    {
      json j = json::object();
      j["text"] = "Filename: " + filename;
      j["icon"] = "feather icon-minus";
      o["children"].push_back(j);
    }
    return o;
    
    
    

};

class VnVProv {
  json toArray(const std::vector<std::shared_ptr<ProvFile>>& array) const;
  void fromArray(std::vector<std::shared_ptr<ProvFile>>& array, const json& a);

 public:
  std::string currentWorkingDirectory;
  long time_in_seconds_since_epoch;
  std::string commandLine;
  std::shared_ptr<ProvFile> inputFile;
  std::shared_ptr<ProvFile> executable;
  std::vector<std::shared_ptr<ProvFile>> inputFiles;
  std::vector<std::shared_ptr<ProvFile>> outputFiles;
  std::vector<std::shared_ptr<ProvFile>> libraries;

  VnVProv(int argc, char** argv, std::string inputfileName, json& config);

  json toJson() const;

  void getDataChildren() const {
    json j = json::object();
    j["text"] = "provenance";
    j["icon"] = "feather icon-folder";
  
    ch = json::array();
    
    json time_in_secs = json::object();
    time_in_secs["text"] = "Unix Start Time: " + std::to_string(time_in_seconds_since_epoch) + " seconds";
    time_in_secs["icon"] = "feather icon-watch";
    ch.push_back(time_in_secs);


    json wdir = json::object();
    wdir["text"] = "Working Directory: " + currentWorkingDirectory;
    wdir["icon"] = "feather icon-text";
    ch.push_back(wdir);

    json commandLine = json::object();
    commandLine["text"] = "Command Line: " + commandLine ;
    commandLine["icon"] = "feather icon-text";
    ch.push_back(commandLine);

    if (inputFile != nullptr) {
      ch.push_back(inputFile->getAsDataChild());
      ch.back()["text"] = "Input File: " + ch.back()["text"];

    }     
    if (executable != nullptr) {
      ch.push_back(executable->getAsDataChild());
      ch.back()["text"] = "Executable: " + ch.back()["text"];

    }     

    std::vector<std::string> names = {"Application Input Files", "Application Output Files", "Shared Libraries" };
    std::vector< std::vector<std::shared_ptr<ProvFile>>*> vals = {&inputFiles,&outputFiles,&libraries};
    for (int i = 0; i < names.size(); i++ ) {
        std::vector<std::shared_ptr<ProvFile>>& data = *(vals[i]);

        if (data.size() > 0 ) {
            json inputs = json::object();
            inputs["text"] = names[i];
            inputs["icon"] = "feather icon-list";
      
            int count = 0;
            json child = json::array();
            for (auto &it : inputFiles) {
              child.push_back(it->getAsDataChild());
              child.back()["text"] = std::to_string(count++) + ": " + child.back()["text"];
            }
            inputs["children"] = child;
            ch.push_back(inputs);
         }
    }






  }

  VnVProv();

  VnVProv(const json& j);

  void addInputFile(std::shared_ptr<ProvFile> pv);

  void addOutputFile(std::shared_ptr<ProvFile> pv);

  std::shared_ptr<ProvFile> get(std::size_t index, int input) {
    return (input == 0) ? inputFiles[index] : (input == 1 ? outputFiles[index] : libraries[index]);
  }

  std::size_t size(int input) {
    return input == 0 ? inputFiles.size() : (input == 1 ? outputFiles.size() : libraries.size());
  }

  virtual ~VnVProv() {}
};

}  // namespace VnV

#endif