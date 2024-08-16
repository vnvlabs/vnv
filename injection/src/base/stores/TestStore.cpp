
/** @file TestStore.cpp **/


#include <dlfcn.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "base/stores/TestStore.h"
#include "shared/Utilities.h"
#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/all.h"
#include "validate/json-schema.hpp"

using namespace VnV;
using nlohmann::json_schema::json_validator;

BaseStoreInstance(TestStore)

void TestStore::addTest(std::string package, std::string name, std::string s, maker_ptr m) {
    VnV_Debug(VNVPACKAGENAME, "Adding a new Test %s:%s", package.c_str(), name.c_str());

    json j = json::parse(s);
    TestInfo k(m, j);
    test_factory.insert({package + ":" + name, k});
  }

  // TODO -- THIS SHOULD RETURN THE VALIDATION ERROR SOMEHOW>
  bool TestStore::verifySchema(std::string package, std::string name, json& opts) {
    // Validate the test configuration provided to the constructor.
    json_validator validator;
    auto sch = test_factory.find(package + ":" + name);
    if (sch != test_factory.end()) {
      validator.set_root_schema(sch->second.schema);
      try {
        validator.validate(opts);
        return true;
      } catch (std::exception& e) {
        return false;
      }
    }
    return true;
  }

  std::shared_ptr<ITest> TestStore::getTest(TestConfig& config) {
    std::string key = config.getPackage() + ":" + config.getName();

    auto it = test_factory.find(key);
    if (it != test_factory.end()) {
      ITest* t = (*it->second.maker)(config);
      std::shared_ptr<ITest> ptr;
      ptr.reset(t);
      return ptr;
    }
    return nullptr;
  }

  nlohmann::json& TestStore::defaultSchemaJson() {
    static json defaultJson = json::parse(R"({"type","object"})");
    return defaultJson;
  }

  std::vector<TestConfig> TestStore::validateTests(std::vector<json>& configs) {
    std::vector<TestConfig> conf;
    for (auto& it : configs) {
      try {
        conf.push_back(validateTest(it));
      } catch (...) {
        VnV_Error(VNVPACKAGENAME, "Test Validation Failed for %s", it.dump().c_str());
      }
    }
    return conf;
  }

  TestConfig TestStore::validateTest(json& testJson) {
    if (testJson.find("name") == testJson.end()) {
      // This should be impossible. Input Validation should detect test blocks
      // incorrectly specified.
      throw "Error During Test Validation: Test Declaration does not contain Test Name";
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

        json& sch = getSchema(package, name);
        test_schema = getTestValidationSchema(sch);
        registeredTests.insert(std::make_pair(key, test_schema));
      } else {
        test_schema = itt->second;
      }

      // Validate the config file.
      json_validator validator;
      validator.set_root_schema(test_schema);

      // Test the test config, and populate it in the case that the parameters,
      // configuration and expectedResult were not provided. This allows users
      // to leave out the configuration blocks when defining a test. The test
      // itself will define if that is a valid configuration.
      json testConfigJson = json::object();
      if (testJson.contains("config")) {
        testConfigJson["configuration"] = testJson["config"];
      } else {
        testConfigJson["configuration"] = json::object();
      }

      validator.validate(testConfigJson);

      // Finally, if a config spec was added for the test
      TestConfig f(package, name, testConfigJson);

      if (testJson.contains("template")) {
        f.runTemplateName = testJson["template"];
      }

      return f;
    }
    throw "Error During Test Validation: Test does not exist.";
  }

  json& TestStore::getSchema(std::string package, std::string name) {
    auto it = test_factory.find(package + ":" + name);
    if (it != test_factory.end()) {
      return it->second.schema;
    } else {
      return defaultSchemaJson();
    }
  }

  nlohmann::json TestStore::schema(json& packageJson) {
    nlohmann::json props = json::object();
    for (auto& it : test_factory) {
      std::vector<std::string> a;
      StringUtils::StringSplit(it.first, ":", a);

      nlohmann::json properties = json::object();
      properties["config"] = it.second.schema;

      json p = json::object();
      p["type"] = "object";
      p["properties"] = properties;
      p["additionalProperties"] = false;
      p["description"] = packageJson[a[0]]["Tests"][a[1]]["docs"]["description"];
      props[it.first] = p;
    }

    json r = json::object();
    r["type"] = "object";
    r["properties"] = props;
    r["additionalProperties"] = false;
    return r;
  }

  void TestStore::print() {}
