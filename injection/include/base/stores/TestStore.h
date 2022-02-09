
/**
  @file TestStore.h
**/
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER

/** @file vv-testing.h
 */

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/parser/JsonSchema.h"
#include "base/stores/BaseStore.h"
#include "interfaces/ITest.h"

namespace VnV {

template <typename V> class TestInfoTemplate {
 public:
  V maker;
  json schema;

  TestInfoTemplate(V ptr, nlohmann::json s) {
    maker = ptr;
    schema = s;
  }
};

/**
 * @brief The TestStore class
 */
template <typename Inter, typename Maker, typename Config> class TestStoreTemplate {
 public:
  TestStoreTemplate() {}

  std::map<std::string, json> registeredTests;

  std::map<std::string, TestInfoTemplate<Maker>, std::less<std::string>> test_factory;

  void addTest(std::string package, std::string name, std::string s, Maker m) {
    VnV_Debug(VNVPACKAGENAME, "Adding a new Test %s:%s", package.c_str(), name.c_str());

    json j = json::parse(s);
    TestInfoTemplate<Maker> k(m, j);
    test_factory.insert({package + ":" + name, k});
  }

  // TODO -- THIS SHOULD RETURN THE VALIDATION ERROR SOMEHOW>
  bool verifySchema(std::string package, std::string name, json& opts) {
    // Validate the test configuration provided to the constructor.
    json_validator validator;
    auto sch = test_factory.find(package + ":" + name);
    if (sch != test_factory.end()) {
      validator.set_root_schema(sch->second.schema);
      try {
        validator.validate(opts);
        return true;
      } catch (std::exception &e) {
        return false;
      }
    }
    return true;
  }

  std::shared_ptr<Inter> getTest(Config& config) {
    std::string key = config.getPackage() + ":" + config.getName();

    auto it = test_factory.find(key);
    if (it != test_factory.end()) {
      Inter* t = (*it->second.maker)(config);
      std::shared_ptr<Inter> ptr;
      ptr.reset(t);
      return ptr;
    }
    return nullptr;
  }

  nlohmann::json& defaultSchemaJson() {
    static json defaultJson = json::parse(R"({"type","object"})");
    return defaultJson;
  }

  std::vector<Config> validateTests(std::vector<json>& configs) {
    std::vector<Config> conf;
    for (auto& it : configs) {
      try {
        conf.push_back(validateTest(it));
      } catch (VnVExceptionBase& e) {
        VnV_Error(VNVPACKAGENAME, "Test Validation Failed for %s  --- %s", it.dump().c_str(), e.what());
      }
    }
    return conf;
  }

  Config validateTest(json& testJson) {
    if (testJson.find("name") == testJson.end()) {
      // This should be impossible. Input Validation should detect test blocks
      // incorrectly specified.
      HTHROW INJECTION_EXCEPTION("Error During Test Validation: Test Declaration does not contain Test Name\n %s",
                                 testJson.dump().c_str());
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
      Config f(package, name, testConfigJson);

      if (testJson.contains("template")) {
        f.runTemplateName = testJson["template"];
      }

      return f;
    }
    HTHROW INJECTION_EXCEPTION("Error During Test Validation: Teset %s:%s does not exist.", package.c_str(),
                               name.c_str());
  }

  json& getSchema(std::string package, std::string name) {
    auto it = test_factory.find(package + ":" + name);
    if (it != test_factory.end()) {
      return it->second.schema;
    } else {
      return defaultSchemaJson();
    }
  }

  nlohmann::json schema() {
    nlohmann::json props = json::object();
    for (auto& it : test_factory) {
      nlohmann::json properties = json::object();
      properties["config"] = it.second.schema;

      json p = json::object();
      p["type"] = "object";
      p["properties"] = properties;
      p["additionalProperties"] = false;
      props[it.first] = p;
    }

    json r = json::object();
    r["type"] = "object";
    r["properties"] = props;
    r["additionalProperties"] = false;
    return r;
  }

  /**
   * @brief print out test store configuration information.
   */
  void print() {
    for (auto it : test_factory) {
    }
  }
};

class TestStore : public TestStoreTemplate<ITest, maker_ptr, TestConfig>, public BaseStore {
 public:
  TestStore() : TestStoreTemplate<ITest, maker_ptr, TestConfig>() {}
  static TestStore& instance();
};

}  // namespace VnV

#endif
