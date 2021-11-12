#ifndef INJECTIONPOINT_CONFIG_H
#define INJECTIONPOINT_CONFIG_H

#include <string>
#include <vector>
#include "base/TestConfig.h"
#include "json-schema.hpp"

namespace VnV {

using nlohmann::json;

class InjectionPointSpec {
 public:
  std::string package;
  std::string name;
  json specJson;
  InjectionPointSpec(std::string package, std::string name, json& spec) {
    this->package = package;
    this->name = name;
    this->specJson = spec;
  }
};

class InjectionPointConfig {
 public:
  std::string packageName;
  std::string name;
  bool runInternal;
  json runConfig = json::object();

  std::vector<TestConfig> tests;

  InjectionPointConfig(std::string package, std::string id, bool runInternal_, json& runConfig_, std::vector<TestConfig>& tests_)
      : packageName(package), name(id), runInternal(runInternal_), tests(tests_), runConfig(runConfig_) {
  }
 

};


}
 // namespace InjectionPointTypeUtils



#endif
