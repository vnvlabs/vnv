
/** @file JsonParser.cpp Implementation of the JsonParser class as defined in
    base/JsonParser.h
**/
#include "base/parser/JsonParser.h"  // Prototype

#include <fstream>

#include "base/parser/JsonSchema.h"  // ValidationSchema()
#include "base/Utilities.h"
#include "base/exceptions.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

void JsonParser::addTest(const json& testJson, std::vector<json>& testConfigs,
                         std::set<std::string>& runScopes) {
  
  if (add(testJson, runScopes)) {
        testConfigs.push_back(testJson);
  }
}

bool JsonParser::add(const json& testJson,
                         std::set<std::string>& runScopes) {
  
  if (runScopes.size() != 0) {
    bool add = false;
    if (testJson.contains("runScope")) {
       for (auto& scope : testJson["runScope"].items()) {
           if (runScopes.find(scope.value().get<std::string>()) != runScopes.end()) {
             add = true;
             break;
           }
       }
    }
    return add;
  }
  return true;
}


void JsonParser::addInjectionPoint(
    const json& ip, std::set<std::string>& runScopes,
    std::map<std::string, InjectionPointInfo>& ips, InjectionType type) {
  for (auto& it : ip.items()) {

    if (!add(it.value(),runScopes)) {
        continue;
    }

    std::string name = it.value()["name"].get<std::string>();
    std::string package = it.value()["package"].get<std::string>();
    std::string key = package + ":" + name;
    const json& values = it.value();
    auto aip = ips.find(name);
    if (aip != ips.end()) {
      if (values.find("tests") != values.end() ) {
        for (auto& test : values["tests"].items()) {
          addTest(test.value(), aip->second.tests, runScopes);
        }
      }
      if (values.find("iterators") != values.end()) {
        for (auto &test : values["iterators"].items()) {
           addTest(test.value(), aip->second.iterators, runScopes);
        }
      }
      if (values.find("plug") != values.end()) {
          json c = values["plug"];
          if (add(c,runScopes)) {
            aip->second.plug = c;
          }
      }
    } else {
      InjectionPointInfo ipInfo;
      ipInfo.type = type;
      ipInfo.name = name;
      ipInfo.package = package;
      if (values.find("tests") != values.end() ) {
            for (auto test : values["tests"].items()) {
              addTest(test.value(), ipInfo.tests, runScopes);
            }
      }
      if (type == InjectionType::ITER && values.find("iterators") != values.end() ) {
        for (auto &test : it.value()["iterators"].items()) {
          addTest(test.value(), ipInfo.iterators, runScopes);
        }
      }
      if (type == InjectionType::PLUG && values.find("plug") != values.end()) {
          json c = values["plug"];
          if (add(c,runScopes)) {
            aip->second.plug = c;
          }
      }
      if (values.contains("runInternal")) {
        ipInfo.runInternal = values["runInternal"].get<bool>();
      } else {
        ipInfo.runInternal = true;
      }

      if (ipInfo.tests.size() > 0 || ipInfo.runInternal || ipInfo.iterators.size() > 0) {
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

OffloadInfo JsonParser::getOffloadInfo(const json& offloadJson) {
   OffloadInfo oinfo;
   oinfo.on = (offloadJson.contains("on")) ? offloadJson["on"].get<bool>() : false;
   if (oinfo.on) {
        oinfo.offloadType = (offloadJson.contains("type")) ? offloadJson["type"].get<std::string>() : "";
        oinfo.offloadConfig = (offloadJson.contains("config"))?offloadJson["config"] : json::object();
   }
   return oinfo;
}

UnitTestInfo JsonParser::getUnitTestInfo(const nlohmann::json& unitTestJson) {
  UnitTestInfo info;
  if (unitTestJson.contains("runUnitTests")) {
    info.runUnitTests = unitTestJson["runUnitTests"].get<bool>();
    info.unitTestConfig = unitTestJson["config"];
  } else {
    info.runUnitTests = false;
    info.unitTestConfig = json::object();
  }

  if ( unitTestJson.contains("exitAfter")) {
     info.exitAfterTests = unitTestJson["exitAfter"].get<bool>();
  } else {
     info.exitAfterTests = false;
  }

  return info;
}


ActionInfo JsonParser::getActionInfo(const nlohmann::json& actionJson) {
  ActionInfo info;
  info.run = actionJson.contains("run") ?  actionJson["run"].get<bool>() : true ;
  if (actionJson.contains("config")) {
      for (auto it : actionJson["config"].items()) {
         ActionConfig config;
         config.name = it.value()["name"].get<std::string>();
         config.package = it.value()["package"].get<std::string>();
         config.run = it.value().contains("run") ? it.value()["run"].get<bool>() : true;
         config.config = it.value().contains("config") ? it.value()["config"] :  json::object();
         info.actions.push_back(std::move(config));
      }  
  }
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

  if (main.find("pluginConfig") != main.end()) {
    info.pluginConfig = main.find("pluginConfig").value();
  }
  info.cmdline = commandLineParser(argc, argv);

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

  if (main.find("offloading") != main.end()) {
    info.offloadInfo = getOffloadInfo(main["offloading"]);
  } else {
    info.offloadInfo = getOffloadInfo(R"({"on" : false })"_json);
  }

  // Get the output Engine information.
  if (main.find("unit-testing") != main.end()) {
    info.unitTestInfo = getUnitTestInfo(main["unit-testing"]);
  } else {
    info.unitTestInfo =
        getUnitTestInfo(R"({"runTests" : false , "config" : {} })"_json);
  }

  if (main.contains("actions")) {
     info.actionInfo = getActionInfo(main["actions"]);
  } else {
    info.actionInfo = { false, {}};
  }


  // Get the test libraries infomation.
  if (main.find("additionalPlugins") != main.end())
    addTestLibrary(main["additionalPlugins"], info.additionalPlugins);

  // Add all the injection points;
  if (main.find("injectionPoints") != main.end()) {
    addInjectionPoint(main["injectionPoints"], runScopes, info.injectionPoints, InjectionType::POINT);
  }
 
  // Add all the injection points;
  if (main.find("iterators") != main.end()) {
    addInjectionPoint(main["iterators"], runScopes, info.injectionPoints, InjectionType::ITER);
  }

  // Add all the injection points;
  if (main.find("plugs") != main.end()) {
    addInjectionPoint(main["plugs"], runScopes, info.injectionPoints, InjectionType::PLUG);
  }



  return info;
}

json JsonParser::commandLineParser(int* argc, char** argv) {
  json main = json::object();
  for (int i = 0; i < *argc; i++) {
    std::string s(argv[i]);
    std::vector<std::string> result;
    StringUtils::StringSplit(s, ".", result);

    // valid parameters are --vnv.packageName.key <value>
    if (result.size() >= 3 && result[0].compare("--vnv") == 0) {
      json& j = JsonUtilities::getOrCreate(main, result[1],
                                           JsonUtilities::CreateType::Object);

      // Set the value to be argv[i+1], the next token in the command line.
      // A bit hacky, but don't set i+=1 to skip the next parameter. This
      // allows for parameters where there is no value. We could not know
      // that without pre-registration, which we should probably do, but
      // this works for now.
      j[result[2]] = (i + 1 == *argc) ? "" : argv[i + 1];
    }
  }
  return main;
}

RunInfo JsonParser::parse(std::ifstream& fstream, int* argc, char** argv) {
  json mainJson;
  if (!fstream.good()) {
    throw VnVExceptionBase(
        "Invalid Input File Stream. The input file stream passed "
        "to JsonParser::parse could not be found and/or opened");
  }

  try {
    mainJson = json::parse(fstream);
  } catch (json::parse_error e) {
    throw Exceptions::parseError(fstream, e.byte, e.what());
  }
  return parse(mainJson, argc, argv);
}

#include <iostream>
RunInfo JsonParser::parse(const json& _json, int* argc, char** argv) {
  json_validator validator;
  validator.set_root_schema(getVVSchema());
  try {
    validator.validate(_json);
  } catch (std::exception e) {
    throw VnVExceptionBase(e.what());
  }
  return _parse(_json, argc, argv);
}
