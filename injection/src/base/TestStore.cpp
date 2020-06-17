
/** @file TestStore.cpp **/

#include "base/TestStore.h"

#include <dlfcn.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "base/JsonSchema.h"
#include "base/OutputEngineStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
using nlohmann::json_schema::json_validator;

using namespace VnV;

nlohmann::json &TestStore::defaultSchemaJson() {
  static json defaultJson = json::parse(R"({"type","object"})");
  return defaultJson;
}

TestStore::TestStore() {}

TestStore& TestStore::getTestStore() {
  static TestStore store;
  return store;
}

void TestStore::addTest(std::string package, std::string name, maker_ptr m,
                        std::map<std::string, std::string> v) {
  test_factory[package + ":" + name] = std::make_pair(m, v);
}

void TestStore::addSchema(std::string package, std::string name, nlohmann::json &schema) {
  schema_factory[package+":"+name] = schema;
}

bool TestStore::verifySchema(std::string package, std::string name, nlohmann::json &opts) {
  // Validate the test configuration provided to the constructor.
  json_validator validator;
  auto sch = schema_factory.find(package+":"+name);
  if (sch!=schema_factory.end()){
      validator.set_root_schema(sch->second);
      try {
        validator.validate(opts);
        return true;
      } catch (std::exception e) {
        return false;
        throw VnVExceptionBase(e.what());
      }
    }
  return true;
}

std::vector<TestConfig> TestStore::validateTests(std::vector<json>& configs) {
  std::vector<TestConfig> conf;
  for (auto& it : configs) {
      conf.push_back(validateTest(it));
    }
  return conf;
}

TestConfig TestStore::validateTest(json& testJson) {
  if (testJson.find("name") == testJson.end()) {
      // This should be impossible. Input Validation should detect test blocks
      // incorretly specified.
      throw VnVExceptionBase("Test Declaration does not contain Test Name");
    }
  std::string name = testJson["name"].get<std::string>();
  std::string package = testJson["package"].get<std::string>();
  std::string key = package + ":" + name;
  auto it = test_factory.find(key);
  if (it != test_factory.end()) {
    json test_schema;
    auto itt = registeredTests.find(key);
    if (itt == registeredTests.end()) {
      // THe test is not yet registered.

      json& sch = getSchema(package,name);
      test_schema = getTestValidationSchema(it->second.second, sch);
      registeredTests.insert(std::make_pair(key, test_schema));
   } else {
      test_schema = itt->second;
   }

    // Validate the config file.
    json_validator validator;
    validator.set_root_schema(test_schema);

    // Test the test config, and populate it in the case that the parameters,
    // configuration and expectedResult were not provided. This allows users to
    // leave out the configuration blocks when defining a test. The test itself
    // will define if that is a valid configuration.
    json testConfigJson;
    auto itc = testJson.find("config");
    if (itc == testJson.end()) {
      testConfigJson = R"({})";
    } else {
      testConfigJson = *itc;
    }
    if (testConfigJson.find("configuration") == testConfigJson.end()) {
      testConfigJson["configuration"] = R"({})"_json;
    }
    if (testConfigJson.find("parameters") == testConfigJson.end()) {
      testConfigJson["parameters"] = R"({})"_json;
    }
    validator.validate(testConfigJson);

    //Finally, if a config spec was added for the test


    return TestConfig(package, name, testConfigJson, it->second.second);
  }
  throw VnVExceptionBase("test not found");
}

nlohmann::json &TestStore::getSchema(std::string package, std::string name) {
  auto it = schema_factory.find(package + ":" + name);
  if (it != schema_factory.end()) {
      return it->second;
    } else {
      return defaultSchemaJson();
    }
}

std::shared_ptr<ITest> TestStore::getTest(TestConfig& config) {
  std::string key = config.getPackage() + ":" + config.getName();

  auto it = test_factory.find(key);
  if (it != test_factory.end()) {
      ITest* t = it->second.first(config);
      std::shared_ptr<ITest> ptr;
    ptr.reset(t);
    return ptr;
  }
  return nullptr;
}

void TestStore::print() {
  auto a = VnV_BeginStage("Test Store Configuration");

  auto b = VnV_BeginStage("Loaded Tests");
  for (auto it : test_factory) {
    auto c = VnV_BeginStage("Test: %s", it.first.c_str());
    VnV_EndStage(c);
  }
  VnV_EndStage(b);
  VnV_EndStage(a);
}

void VnV::registerTest(std::string package, std::string name, maker_ptr m,
                       std::map<std::string, std::string> map) {
  for (auto op : map)
    std::cout << "TEST P " << op.first << " :  " << op.second << std::endl;
  TestStore::getTestStore().addTest(package, name, m, map);
}
void VnV::registerTestSchema(std::string package, std::string name, std::string schema) {
  json j = json::parse(schema);
  TestStore::getTestStore().addSchema(package,name,j);
}

