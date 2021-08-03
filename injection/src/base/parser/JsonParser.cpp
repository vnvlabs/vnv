
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


SamplerInfo JsonParser::getSamplerInfo(const json& samplerJson) {
   SamplerInfo info;
   info.name = samplerJson["name"].get<std::string>();
   info.package = samplerJson["package"].get<std::string>();
   info.config = samplerJson.contains("config") ? samplerJson["config"] : json::object();
   return info;
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

      if (values.find("sampler") != values.end()) {
          aip->second.sampler = getSamplerInfo(values["sampler"]);
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
      if (values.find("sampler") != values.end()) {
          ipInfo.sampler = getSamplerInfo(values["sampler"]);
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



UnitTestInfo JsonParser::getUnitTestInfo(const nlohmann::json& unitTestJson) {
  UnitTestInfo info;
  if (unitTestJson.contains("runUnitTests")) {
    info.runUnitTests = unitTestJson["runUnitTests"].get<bool>();
    info.unitTestConfig = unitTestJson.contains("config") ? unitTestJson["config"] : json::object() ;
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


ActionInfo JsonParser::getActionInfo(const nlohmann::json& actionJson, std::string type) {
  ActionInfo info;
  info.run = true;
  for (auto it : actionJson.items()) {
         ActionConfig config;
         size_t colon_pos = it.key().find(':');
         config.package = it.key().substr(0,colon_pos);
         config.name = it.key().substr(colon_pos+1); 
         config.config = it.value();
         config.run = type;
         info.actions.push_back(std::move(config));
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
    for (auto& it : main["runScopes"].items()) {
        runScopes.insert(it.value().get<std::string>());
      }
  }
  
  // Get the output Engine information.
  if (main.find("outputEngine") != main.end()) {
    info.engineInfo = getEngineInfo(main["outputEngine"]);
  } else {
    info.engineInfo =
        getEngineInfo(R"({"type" : "json_stdout" , "config" : {} })"_json);
  }

  // Get the output Engine information.
  if (main.find("unit-testing") != main.end()) {
    info.unitTestInfo = getUnitTestInfo(main["unit-testing"]);
  } else {
    info.unitTestInfo =
        getUnitTestInfo(R"({"runTests" : false , "config" : {} })"_json);
  }

  info.actionInfo = { false, {}};
  if (main.contains("configure-actions")) {
     info.actionInfo.run = true;
     ActionInfo f  = getActionInfo(main["configure-actions"],"configure");
     for (auto it : f.actions) {
       info.actionInfo.actions.push_back(it);
     }
  } 
  if (main.contains("finalize-actions")) {
     info.actionInfo.run = true;
     ActionInfo f  = getActionInfo(main["finalize-actions"],"finalize");
     for (auto it : f.actions) {
       info.actionInfo.actions.push_back(it);
     }
  } 
  if (main.contains("communicator")) {
    info.communicator = main["communicator"].get<std::string>();
  } else {
    info.communicator = ""; // Use the default. 
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

  if (main.find("hotpatch") != main.end()) {
    info.hotpatch = main["hotpatch"].get<bool>();
  }

// Add all the injection points;
  if (main.find("template-overrides") != main.end()) {
    info.template_overrides = main["template-overrides"];
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
