#include "base/Provenance.h"

#include <iostream>

#include "base/Utilities.h"

using namespace VnV;

ProvFile::ProvFile() {}

ProvFile::ProvFile(std::string filename, std::string reader, std::string text) {
  this->filename = filename;
  this->reader = reader;
  this->text = text;

  try {
    info = DistUtils::getLibInfo(filename, 0);
  } catch (std::exception& e) {
  }
}

ProvFile::ProvFile(DistUtils::libInfo lb, std::string reader) {
  this->filename = lb.name;
  this->reader = reader;
  this->text = "";
  this->info = lb;
}

ProvFile::ProvFile(const json& j) {
  this->filename = j["filename"].get<std::string>();
  this->reader = j["reader"].get<std::string>();
  this->text = j["text"].get<std::string>();
  this->info.fromJson(j["info"]);
  this->comm = j["comm"].get<long>();
  this->package = j["package"].get<std::string>();
  this->name = j["name"].get<std::string>();
}

json ProvFile::toJson() const {
  json j = json::object();
  j["filename"] = filename;
  j["reader"] = reader;
  j["text"] = text;
  j["info"] = info.toJson();
  j["comm"] = comm;
  j["package"] = package;
  j["name"] = name;
  return j;
}

bool ProvFile::modified() const {
  try {
    DistUtils::libInfo l = DistUtils::getLibInfo(filename, 0);
    return l.timestamp != info.timestamp;
  } catch (std::exception& e) {
    return true;
  }
}

json VnVProv::toArray(const std::vector<std::shared_ptr<ProvFile>>& array) const {
  json a = json::array();
  for (auto it : array) {
    a.push_back(it->toJson());
  }
  return a;
}

void VnVProv::fromArray(std::vector<std::shared_ptr<ProvFile>>& array, const json& a) {
  for (auto& it : a) {
    array.push_back(std::make_shared<ProvFile>(it));
  }
}

VnVProv::VnVProv(int argc, char** argv, std::string inputfileName, json& config)

{
  inputFile.reset(new ProvFile(inputfileName, "json", config.dump(4)));
  executable.reset(new ProvFile(argv[0], "binary"));
  currentWorkingDirectory = DistUtils::getCurrentDirectory();
  time_in_seconds_since_epoch = time(NULL);
  commandLine = VnV::ProvenanceUtils::cmdLineToString(argc, argv);

  DistUtils::libData lb;
  DistUtils::getAllLinkedLibraryData(&lb);
  for (auto it : lb.libs) {
    ProvFile pf(it, "binary");
    libraries.push_back(std::make_shared<ProvFile>(it, "binary"));
  }
}

json VnVProv::toJson() const {
  json j = json::object();

  j["cwd"] = currentWorkingDirectory;
  j["time"] = time_in_seconds_since_epoch;
  j["cmd"] = commandLine;
  j["inp"] = inputFile->toJson();
  j["exe"] = executable->toJson();
  j["inps"] = toArray(inputFiles);
  j["outs"] = toArray(outputFiles);
  j["libs"] = toArray(libraries);
  return j;
}

VnVProv::VnVProv() {}

VnVProv::VnVProv(const json& j) {
  currentWorkingDirectory = j["cwd"].get<std::string>();
  time_in_seconds_since_epoch = j["time"].get<long>();
  commandLine = j["cmd"].get<std::string>();
  inputFile.reset(new ProvFile(j["inp"]));
  executable.reset(new ProvFile(j["exe"]));

  fromArray(inputFiles, j["inps"]);
  fromArray(outputFiles, j["outs"]);
  fromArray(libraries, j["libs"]);
}

void VnVProv::addInputFile(std::shared_ptr<ProvFile> pv) { inputFiles.push_back(pv); }

void VnVProv::addOutputFile(std::shared_ptr<ProvFile> pv) { outputFiles.push_back(pv); }
