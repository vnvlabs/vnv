
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
                         std::vector<TestConfig>& testConfigs,
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

  TestConfig c;
  c.setName(testName);

  // TODO use the tests schema to validate the additional properties;

  if (testJson.find("testConfig") != testJson.end()) {
     std::cout << "GSGDSGD " << testJson["testConfig"].dump() <<std::endl;;
      c.setAdditionalProperties(testJson["testConfig"]);
  } else {
    c.setAdditionalProperties(R"("{}")"_json);
     std::cout << "DDDD" << std::endl;;
  }

  for (auto& stage : testJson["stages"].items()) {
    TestStageConfig config;
    config.setTestStageId(std::stoi(stage.key()));
    config.setInjectionPointStageId(stage.value()["ipId"].get<int>());
    for (auto& param : stage.value()["mapping"].items()) {
      std::string trans =
          (param.value().find("transform") != param.value().end())
              ? param.value()["transform"].get<std::string>()
              : "default";
      config.addTransform(
          param.key(), param.value()["ipParameter"].get<std::string>(), trans);
    }
    if (stage.value().find("expected") != stage.value().end()) {
      config.setExpectedResult(stage.value()["expected"]);
    } else {
      config.setExpectedResult(R"({})"_json);
    }
    c.addTestStage(config);
  }
  testConfigs.push_back(c);
}

void JsonParser::addInjectionPoint(
    const json& ip, std::set<std::string>& runScopes,
    std::map<std::string, std::vector<TestConfig>>& ips) {
  json ipd = ip["config"];
  for (auto& it : ipd.items()) {
    std::string name = it.value()["name"].get<std::string>();

    auto aip = ips.find(name);
    if (aip != ips.end()) {
      for (auto& test : it.value()["tests"].items()) {
        addTest(test.value(), aip->second, runScopes);
      }
    } else {
      std::vector<TestConfig> testConfigs;
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

void JsonParser::setupLogger(const json& logging) {
  if (logging["on"].get<bool>()) {
    std::string type = logging["filename"].get<std::string>();
    VnV_ConfigureLog(logging["filename"].get<std::string>());
    for (auto& it : logging["logs"].items()) {
      VnV_ConfigureLogLevel(it.key(), it.value().get<bool>());
    }
  }
}

EngineInfo JsonParser::getEngineInfo(const json& engine) {
  EngineInfo einfo;
  einfo.engineType = engine["type"].get<std::string>();
  einfo.engineConfig = engine["config"];
  return einfo;
}

RunInfo JsonParser::parse(const json& main) {
  // First port of call is to set up logging. This is done here, so we can
  // use logging inside the parser itself.
  if (main.find("logging") != main.end())
    setupLogger(main["logging"]);  // getLoggingInfo(main["logging"])
  else
    setupLogger(R"({"on" : true, "filename" : "stdout", "logs" : {} })"_json);

  RunInfo info;

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
        R"({"outputFile" : "./vv.out" , "debug" : true , "type" : "debug" , "configFile" : "" })"_json);
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

RunInfo JsonParser::parse(std::string filename) {
  // TODO -- Change input parameter to represent the fact that this function
  // supports passing in
  // a string filename, and a string representation of a json object.
  //
  // Currently the code tries to open "filename" as a file. If that fails, it
  // parses it as a json string. If that fails, it throws an invalid json error.

  std::ifstream input(filename);

  json mainJson;
  if (!input) {
    mainJson = json::parse(filename);
  } else {
    mainJson = json::parse(input);
  }

  json_validator validator;
  validator.set_root_schema(vv_schema);
  validator.validate(mainJson);
  return parse(mainJson);
}
