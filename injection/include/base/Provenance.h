#ifndef PROVANENCE_HEADER
#define PROVANENCE_HEADER

#include <string>
#include <vector>
#include "base/DistUtils.h"
#include "json-schema.hpp"
using nlohmann::json;

namespace VnV {

class ProvFile {
 public:
  std::string filename;
  std::string reader;
  std::string text = "";
  std::string package = "root";
  std::string name = "<none>";
  long comm = -1;

  VnV::DistUtils::libInfo info;

  ProvFile();
  ProvFile(std::string filename, std::string reader, std::string text = "");
  ProvFile(VnV::DistUtils::libInfo lb, std::string reader);

  ProvFile(const json& j);
  json toJson();

  bool modified();
};

class VnVProv {

  json toArray(std::vector<ProvFile>& array);
  void fromArray(std::vector<ProvFile>& array, const json& a);

 public:


  std::string currentWorkingDirectory;
  long time_in_seconds_since_epoch;
  std::string commandLine;
  ProvFile inputFile;
  ProvFile executable;
  std::vector<ProvFile> inputFiles;
  std::vector<ProvFile> outputFiles;
  std::vector<ProvFile> libraries;



  VnVProv(int argc, char** argv, std::string inputfileName, json& config);

  json toJson();
  VnVProv();

  VnVProv(const json& j);

  void addInputFile(ProvFile pv);

  void addOutputFile(ProvFile pv);

  
};

}  // namespace VnV

#endif