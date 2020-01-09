
/** @file vv-parser.cpp **/

#include "vv-parser.h"

#include <fstream>
#include <iostream>
#include <string>

#include "vv-injection.h"
#include "vv-logging.h"
#include "vv-output.h"
#include "vv-schema.h"
#include "vv-testing.h"
#include "vv-utils.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

void JsonParser::addTest(const json& testJson,
                         std::vector<json>& testConfigs,
                         std::set<std::string>& runScopes) {
  std::string testName = testJson["name"].get<std::string>();
  if (runScopes.size() != 0) {
    bool add = false;
    for (auto& scope : testJson["runScope"].items()) {
      if (runScopes.find(scope.value().get<std::string>()) != runScopes.end()) {
        add = true;
        break;
      }
    }
    if (!add) return;
  }
  testConfigs.push_back(testJson);
}

void JsonParser::addInjectionPoint(
    const json& ip, std::set<std::string>& runScopes,
    std::map<std::string, std::vector<json>>& ips) {
  for (auto& it : ip.items()) {
    std::string name = it.value()["name"].get<std::string>();

    auto aip = ips.find(name);
    if (aip != ips.end()) {
      for (auto& test : it.value()["tests"].items()) {
        addTest(test.value(), aip->second, runScopes);
      }
    } else {
      std::vector<json> testConfigs;
      for (auto test : it.value()["tests"].items()) {
        addTest(test.value(), testConfigs, runScopes);
      }

      if (testConfigs.size() > 0) {
        ips.insert(std::make_pair(name, testConfigs));
      }
    }
  }
}

void JsonParser::addTestLibrary(const json& lib, std::set<std::string>& libs) {
  for (auto& it : lib.items()) {
    libs.insert(it.value().get<std::string>());
  }
}

LoggerInfo JsonParser::getLoggerInfo(const json& logging) {

  LoggerInfo info;
  info.on = logging["on"].get<bool>();
  if (info.on) {
    info.filename = logging["filename"].get<std::string>();
    for (auto& it : logging["logs"].items()) {
      info.logs.insert(std::make_pair(it.key(), it.value().get<bool>()));
    }
    auto it = logging.find("blackList");
    if ( it != logging.end() ) {
        for (auto itt : logging["blackList"].items()) {
            info.blackList.insert(itt.value().get<std::string>());
        }
    }

  }
  return info;
}

EngineInfo JsonParser::getEngineInfo(const json& engine) {
  EngineInfo einfo;
  einfo.engineType = engine["type"].get<std::string>();
  einfo.engineConfig = engine["config"];
  return einfo;
}

RunInfo JsonParser::_parse(const json& main) {
  RunInfo info;

  if (main.find("logging") != main.end())
    info.logInfo = getLoggerInfo(main["logging"]);
  else {
      info.logInfo.filename = "stdout";
      info.logInfo.on = true;
  }

  if (main.find("toolConfig") != main.end()) {
      info.toolConfig = main.find("toolConfig").value();
  }

  // Get the run information and the scopes.
  if (main.find("runTests") != main.end()) {
    info.runTests = main["runTests"].get<bool>();
  } else {
    info.runTests = true;
  }
  if (!info.runTests) return info;

  // Get the run Scopes info.
  std::set<std::string> runScopes;
  if (main.find("runScopes") != main.end()) {
    int count = 0;
    for (auto& it : main["runScopes"].items()) {
      if (it.value()["run"].get<bool>()) {
        runScopes.insert(it.value()["name"].get<std::string>());
      }
      count++;
    }
    // Run scopes was defined, all were false
    if (runScopes.size() == 0 && count > 0) {
      info.runTests = false;
      return info;
    }
  }

  // Get the output Engine information.
  if (main.find("outputEngine") != main.end()) {
    info.engineInfo = getEngineInfo(main["outputEngine"]);
  } else {
    info.engineInfo = getEngineInfo(
        R"({"type" : "debug" , "config" : {} })"_json);
  }

  // Get the test libraries infomation.
  if (main.find("testLibraries") != main.end())
    addTestLibrary(main["testLibraries"], info.testLibraries);

  // Add all the injection points;
  if (main.find("injectionPoints") != main.end()) {
    addInjectionPoint(main["injectionPoints"], runScopes, info.injectionPoints);
  }
  return info;
}

RunInfo JsonParser::parse(std::ifstream &fstream) {
  
 	 
  json mainJson;
  if (!fstream.good()) {
    throw "Invalid Input File Stream";
  }

  try {
     mainJson = json::parse(fstream);
  } catch (json::exception e) {
     throw;
  }
  return parse(mainJson);
}

RunInfo JsonParser::parse(const json& _json) {

  json_validator validator;
  validator.set_root_schema(getVVSchema());
  try {
    validator.validate(_json);
  } catch (std::exception e) {
    throw;
  }
  return _parse(_json);
}
