
/** @file JsonParser.cpp Implementation of the JsonParser class as defined in
    base/JsonParser.h
**/

#include "base/parser/JsonParser.h"  // Prototype

#include <fstream>
#include <iostream>

#include "shared/Utilities.h"
#include "shared/exceptions.h"
#include "base/parser/JsonSchema.h"  // ValidationSchema()

using namespace VnV;
using nlohmann::json_schema::json_validator;

void JsonParser::addTest(const std::string key, const json& testJson, std::vector<json>& testConfigs,
                         std::set<std::string>& runScopes) {
  std::vector<std::string> r;
  StringUtils::StringSplit(key, ":", r);
  assert(r.size() == 2 && "Not a valid key");
  json t = testJson;
  t["name"] = r[1];
  t["package"] = r[0];
  if (add(testJson, runScopes)) {
    testConfigs.push_back(t);
  }
}

bool JsonParser::add(const json& testJson, std::set<std::string>& runScopes) {
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
  assert(samplerJson.size() == 1);

  for (auto it : samplerJson.items()) {
    SamplerInfo info;
    std::vector<std::string> s;
    StringUtils::StringSplit(it.key(), ":", s);
    assert(s.size() == 2);
    info.package = s[0];
    info.name = s[1];
    info.config = it.value();
    return info;
  }

  throw VnVExceptionBase("Schema Validation Error -- This should be possible");
}

bool JsonParser::addInjectionPoint(const json& ip, std::set<std::string>& runScopes,
                                   std::map<std::string, InjectionPointInfo>& ips, InjectionType type, std::vector<json>& all_on_tests) {
  bool all_on = false;

  for (auto& it : ip.items()) {
    if (it.key().compare("runAll") == 0) {
      all_on = it.value().get<bool>();
      continue;
    } else if (it.key().compare("runAll_tests") == 0) {
        for (auto &itt : it.value().items()) {
          addTest(itt.key(),itt.value(),all_on_tests,runScopes);
        }
        continue;
    } 
    
    else if (!add(it.value(), runScopes)) {
      continue;
    }

    const json& values = it.value();
    auto aip = ips.find(it.key());

    if (aip != ips.end()) {
      if (values.find("tests") != values.end()) {
        for (auto& test : values["tests"].items()) {
          addTest(test.key(), test.value(), aip->second.tests, runScopes);
        }
      }
     
      if (values.find("sampler") != values.end()) {
        aip->second.sampler = getSamplerInfo(values["sampler"]);
      }

    } else {
      std::vector<std::string> res;
      StringUtils::StringSplit(it.key(), ":", res);
      assert(res.size() == 2);

      InjectionPointInfo ipInfo;
      ipInfo.type = type;
      ipInfo.name = res[1];
      ipInfo.package = res[0];

      if (values.find("tests") != values.end()) {
        for (auto test : values["tests"].items()) {
          addTest(test.key(), test.value(), ipInfo.tests, runScopes);
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

      if (values.contains("template")) {
        ipInfo.templateName = values["template"];
      }

      if (ipInfo.tests.size() > 0 || ipInfo.runInternal ) {
        ips.insert(std::make_pair(it.key(), ipInfo));
      }
    }
  }
  return all_on;
}

void JsonParser::addTestLibrary(const json& lib, std::map<std::string, std::string>& libs) {
  for (auto& it : lib.items()) {
    libs.insert(std::make_pair(it.key(), it.value().get<std::string>()));
  }
}

LoggerInfo JsonParser::getLoggerInfo(const json& logging) {
  LoggerInfo info;
  info.on = logging.value("on", true);

  if (info.on) {
    info.engine = logging.value("engine", true);

    info.filename = logging.value("filename", "vnv-logs.out");
    std::string t = logging.value("type", "stdout");
    if (t.compare("stdout") == 0) {
      info.type = LogWriteType::STDOUT;
    } else if (t.compare("stderr") == 0) {
      info.type = LogWriteType::STDERR;
    } else if (t.compare("file") == 0) {
      info.type = LogWriteType::FILE;
    } else {
      info.type = LogWriteType::NONE;
    }

    if (logging.contains("logs")) {
      for (auto& it : logging["logs"].items()) {
        info.logs.insert(std::make_pair(it.key(), it.value().get<bool>()));
      }
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
  assert(engine.size() == 1 && "Validation should make this always true");

  for (auto it : engine.items()) {
    EngineInfo einfo;
    einfo.engineType = it.key();
    einfo.engineConfig = it.value();
    return einfo;
  }
  throw VnVExceptionBase("Validation Error");
}

UnitTestInfo JsonParser::getUnitTestInfo(const nlohmann::json& unitTestJson) {
  UnitTestInfo info;
  if (unitTestJson.contains("runUnitTests")) {
    info.runUnitTests = unitTestJson["runUnitTests"].get<bool>();
    info.unitTestConfig = unitTestJson.contains("config") ? unitTestJson["config"] : json::object();
  } else {
    info.runUnitTests = false;
    info.unitTestConfig = json::object();
  }

  if (unitTestJson.contains("exitAfter")) {
    info.exitAfterTests = unitTestJson["exitAfter"].get<bool>();
  } else {
    info.exitAfterTests = false;
  }

  return info;
}

ActionInfo JsonParser::getActionInfo(const nlohmann::json& actionJson) {
  ActionInfo info;
  info.run = true;
  for (auto it : actionJson.items()) {
    ActionConfig config;
    size_t colon_pos = it.key().find(':');
    config.package = it.key().substr(0, colon_pos);
    config.name = it.key().substr(colon_pos + 1);
    config.config = it.value();
    config.run = true;
    info.actions.push_back(std::move(config));
  }
  return info;
}

namespace {

// We accept arguments of the type
// "vnv/options/sdfsdf/sdfsdf/sdfsdf/=valid_json"
nlohmann::json updateFileWithCommandLineOverrides(const json& mainFile, int* argc, char** argv) {
  nlohmann::json main = mainFile;

  for (int i = 0; i < *argc; i++) {
    std::string s = argv[i];
    try {
      if (s.substr(0, 6).compare("--vnv/") == 0) {
        std::size_t ind = s.find_first_of("=");
        if (ind != std::string::npos) {
          std::string point = s.substr(6, ind);
          std::string ans = s.substr(ind + 1);
          json ansJson;
          json::json_pointer ptr;

          try {
            json ansJson = json::parse(ans);
            try {
              json::json_pointer ptr = json::json_pointer(point);
              if (main.contains(ptr)) {
                main[ptr] = ansJson;
              } else {
                throw INJECTION_EXCEPTION_("Adding new values is not supported: ");
              }
            } catch (std::exception& e) {
              throw INJECTION_EXCEPTION("Invalid Json Pointer: %s", point.c_str());
            }

          } catch (std::exception& e) {
            throw INJECTION_EXCEPTION("Invalid Json %s", ans.c_str());
          }

        } else {
          throw INJECTION_EXCEPTION_("Invalid VnV Command line argument");
        }
      }
    } catch (VnVExceptionBase& e) {
      std::cout << "Ignoring VnV command line parameter : " << s << "\n Reason: " << e.message << std::endl;
    }
  }
  return main;
}

void addCommandLinePlugins(int* argc, char** argv, RunInfo& info) {
  int i = 0;
  while (i < *argc) {
    std::string s = argv[i++];
    if (s.compare("--vnv-plugin") == 0) {
      if (i + 1 < *argc) {
        std::string pname = argv[i++];
        std::string file = argv[i++];
        info.additionalPlugins[pname] = file;
      } else {
        throw INJECTION_EXCEPTION_("Invalid Command line plugin");
      }
    }
  }
}

}  // namespace

WorkflowInfo JsonParser::getWorkflowInfo(const json& workflowInfo) {
  WorkflowInfo info = {true, true, {}};

  info.run = workflowInfo.value("run", true);
  info.quit = workflowInfo.value("quit", true);

  for (auto& it : workflowInfo.items()) {
    if (it.key().compare("run") == 0) {
      info.run = it.value().get<bool>();
    } else if (it.key().compare("quit") == 0) {
      info.quit = it.value().get<bool>();
    } else {
      std::vector<std::string> r;
      StringUtils::StringSplit(it.key(), ":", r);
      if (r.size() == 2) {
        info.workflows.push_back({r[0], r[1], it.value()});
      } else {
        throw VnVExceptionBase("Validation should catch this");
      }
    }
  }
  return info;
}

RunInfo JsonParser::_parse(const json& mainFile, int* argc, char** argv) {
  json main = updateFileWithCommandLineOverrides(mainFile, argc, argv);

  RunInfo info;
  if (main.find("logging") != main.end())
    info.logInfo = getLoggerInfo(main["logging"]);
  else {
    info.logInfo.filename = "stdout";
    info.logInfo.on = true;
  }

  if (main.find("job") != main.end()) {
    info.workflowDir = main["job"].value("dir", "/tmp");
  }

  if (main.find("schema") != main.end()) {
    info.schemaDump = main["schema"].value("dump", false);
    info.schemaQuit = main["schema"].value("quit", false);
  }

  if (main.find("options") != main.end()) {
    info.pluginConfig = main.find("options").value();
  } else {
    info.pluginConfig = json::object();
  }

  info.cmdline = commandLineParser(argc, argv);

  // Get the run information and the scopes.
  if (main.find("runTests") != main.end()) {
    info.runTests = main["runTests"].get<bool>();
  } else {
    info.runTests = false;
  }
// Get the run information and the scopes.
  if (main.find("name") != main.end()) {
    info.name = main["name"].get<std::string>();
  } else {
    info.name = "";
  }

  if (main.find("description") != main.end()) {
    info.description = main["description"].get<std::string>();
  } else {
    info.description = "";
  }

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
    info.engineInfo = getEngineInfo(R"({ "stdout" : {} })"_json);
  }

  // Get the output Engine information.
  if (main.find("unit-testing") != main.end()) {
    info.unitTestInfo = getUnitTestInfo(main["unit-testing"]);
  } else {
    info.unitTestInfo = getUnitTestInfo(R"({"runTests" : false , "config" : {} })"_json);
  }

  if (main.contains("actions")) {
    info.actionInfo = getActionInfo(main["actions"]);
    info.actionInfo.run = true;
  }

  if (main.contains("communicator")) {
    info.communicator = main["communicator"].get<std::string>();
  } else {
    info.communicator = "";  // Use the default.
  }

  // Get the test libraries infomation.
  if (main.find("additionalPlugins") != main.end()) addTestLibrary(main["additionalPlugins"], info.additionalPlugins);

  // Add any explicity stated command line plugins.
  addCommandLinePlugins(argc, argv, info);

  if (main.find("workflows") != main.end()) {
    info.workflowInfo = getWorkflowInfo(main["workflows"]);
  } else {
    info.workflowInfo.quit = false;
    info.workflowInfo.run = false;
  }

  // Add all the injection points;
  if (main.find("injectionPoints") != main.end()) {
    info.runAll = addInjectionPoint(main["injectionPoints"], runScopes, info.injectionPoints, InjectionType::POINT, info.runAll_tests);
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
    if (s.substr(0,6).compare("--vnv-") == 0) {
      main[s.substr(6)] = (i+1 == *argc) ? "" : argv[i+1];
    }
  }
  return main;
}

RunInfo JsonParser::parse(std::ifstream& fstream, int* argc, char** argv) {
  json mainJson;
  if (!fstream.good()) {
    throw INJECTION_EXCEPTION_(
        "Invalid Input File Stream. The input file stream passed "
        "to JsonParser::parse could not be opened");
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
  } catch (std::exception& e) {
    std::cout << e.what();
    std::cout << getVVSchema().dump(4);
    throw INJECTION_EXCEPTION("Input File Parsing Failed.\n Reason : %s", e.what());
  }
  return _parse(_json, argc, argv);
}
