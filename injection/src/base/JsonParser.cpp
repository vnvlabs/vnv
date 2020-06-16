﻿
/** @file JsonParser.cpp Implementation of the JsonParser class as defined in
    base/JsonParser.h
**/

#include "base/JsonParser.h"  // Prototype

#include "base/JsonSchema.h"  // ValidationSchema()
#include "base/exceptions.h"
#include "base/Utilities.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

void JsonParser::addTest(const json& testJson, std::vector<json>& testConfigs,
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
    std::map<std::string, InjectionPointInfo>& ips) {
  for (auto& it : ip.items()) {
    std::string name = it.value()["name"].get<std::string>();
    std::string package = it.value()["package"].get<std::string>();
    std::string key = package + ":" + name;
    auto aip = ips.find(name);
    if (aip != ips.end()) {
      for (auto& test : it.value()["tests"].items()) {
        addTest(test.value(), aip->second.tests, runScopes);
      }
    } else {
      InjectionPointInfo ipInfo;
      ipInfo.name = name;
      ipInfo.package = package;
      for (auto test : it.value()["tests"].items()) {
        addTest(test.value(), ipInfo.tests, runScopes);
      }
      if (it.value().contains("runInternal")) {
        ipInfo.runInternal = it.value()["runInternal"].get<bool>();
      } else {
        ipInfo.runInternal = true;
      }

      if (ipInfo.tests.size() > 0 || ipInfo.runInternal) {
        ips.insert(std::make_pair(key, ipInfo));
      }
    }
  }
}

void JsonParser::addTestLibrary(const json& lib,
                                std::map<std::string, std::string>& libs) {
  for (auto& it : lib.items()) {
    libs.insert(std::make_pair(it.key(), it.value().get<std::string>()));
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
    if (it != logging.end()) {
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

UnitTestInfo JsonParser::getUnitTestInfo(const nlohmann::json& unitTestJson) {
  UnitTestInfo info;
  info.runUnitTests = unitTestJson["runTests"].get<bool>();
  info.unitTestConfig = unitTestJson["config"];
  return info;
}

RunInfo JsonParser::_parse(const json& main, int* argc, char** argv) {
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
  info.cmdline = commandLineParser(argc,argv);

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
    info.engineInfo =
        getEngineInfo(R"({"type" : "debug" , "config" : {} })"_json);
  }

  // Get the output Engine information.
  if (main.find("unit-testing") != main.end()) {
    info.unitTestInfo = getUnitTestInfo(main["unit-testing"]);
  } else {
    info.unitTestInfo =
        getUnitTestInfo(R"({"runTests" : false , "config" : {} })"_json);
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

json JsonParser::commandLineParser( int* argc, char** argv) {
  json main = json::object();
  for ( int i = 0; i < *argc; i++ ) {
     std::string s(argv[i]);
     std::vector<std::string> result;
     StringUtils::StringSplit(s,".",result);

     // valid parameters are --vnv.packageName.key <value>
     if (result.size() >= 3  && result[0].compare("--vnv") == 0 ) {
        json & j = JsonUtilities::getOrCreate(main,result[1],JsonUtilities::CreateType::Object);

        //Set the value to be argv[i+1], the next token in the command line.
        //A bit hacky, but don't set i+=1 to skip the next parameter. This
        // allows for parameters where there is no value. We could not know
        // that without pre-registration, which we should probably do, but
        // this works for now.
        j[result[2]] = (i+1 == *argc) ? "" : argv[i+1];

      }
  }
  return main;
}



RunInfo JsonParser::parse(std::ifstream& fstream, int *argc, char** argv) {
  json mainJson;
  if (!fstream.good()) {
    throw VnVExceptionBase("Invalid Input File Stream");
  }

  try {
    mainJson = json::parse(fstream);
  } catch (json::exception e) {
    throw VnVExceptionBase(e.what());
  }
  return parse(mainJson,argc,argv);
}

RunInfo JsonParser::parse(const json& _json, int *argc, char** argv) {
  json_validator validator;
  validator.set_root_schema(getVVSchema());
  try {
    validator.validate(_json);
  } catch (std::exception e) {
    throw VnVExceptionBase(e.what());
  }
  return _parse(_json,argc,argv);
}
