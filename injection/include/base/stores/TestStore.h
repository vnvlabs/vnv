
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


#include "interfaces/ITest.h"

namespace VnV {

template <typename V> class TestInfoTemplate {
 public:
  V* maker;
  std::map<std::string, std::string> parameterMap;
  json schema;

  TestInfoTemplate(std::map<std::string, std::string>& p, V* ptr,
                  nlohmann::json s) {
    maker = ptr;
    parameterMap = p;
    schema = s;
  }
};

/**
 * @brief The TestStore class
 */
template <typename Inter, typename Maker, typename Config>
class TestStoreTemplate {
 
 public:
   TestStoreTemplate() {}
 
  std::map<std::string, json> registeredTests;

  std::map<std::string, TestInfoTemplate<Maker>, std::less<std::string>> test_factory;


  void addTest(std::string package, std::string name, std::string s, Maker m,
               std::map<std::string, std::string> v) {
    VnV_Warn(VNVPACKAGENAME, "Adding a new Test %s:%s", package.c_str(), name.c_str());
    json j = json::parse(s);
    test_factory.insert(std::make_pair(package + ":" + name, TestInfoTemplate<Maker>(v, m, j)));
  }

  bool verifySchema(std::string package, std::string name, json& opts) {
    // Validate the test configuration provided to the constructor.
    json_validator validator;
    auto sch = test_factory.find(package + ":" + name);
    if (sch != test_factory.end()) {
      validator.set_root_schema(sch->second.schema);
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

  std::shared_ptr<Inter> getTest(Config& config) {
    std::string key = config.getPackage() + ":" + config.getName();

    auto it = test_factory.find(key);
    if (it != test_factory.end()) {
      Inter* t = it->second.maker(config);
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
      conf.push_back(validateTest(it));
    }
    return conf;
  }

  Config validateTest(json& testJson) {
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

        json& sch = getSchema(package, name);
        test_schema = getTestValidationSchema(it->second.parameterMap, sch);
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
      if (testJson.contains("parameters")) {
        testConfigJson["parameters"] = testJson["parameters"];
      } else {
        testConfigJson["parameters"] = json::object();  
      }

      validator.validate(testConfigJson);

      // Finally, if a config spec was added for the test
      Config f(package, name, testConfigJson, it->second.parameterMap);
      return f;
    }
    throw VnVExceptionBase("test not found");
  }

  json& getSchema(std::string package, std::string name) {
    auto it = test_factory.find(package + ":" + name);
    if (it != test_factory.end()) {
      return it->second.schema;
    } else {
      return defaultSchemaJson();
    }
  }

  /**
   * @brief print out test store configuration information.
   */
  void print() {
    auto b = VnV_BeginStage(VNVPACKAGENAME, "Loaded Tests");
    for (auto it : test_factory) {
      auto c = VnV_BeginStage(VNVPACKAGENAME, "Test: %s", it.first.c_str());
      VnV_EndStage(VNVPACKAGENAME, c);
    }
    VnV_EndStage(VNVPACKAGENAME, b);
  }
};

class TestStore : public  TestStoreTemplate<ITest, maker_ptr, TestConfig> {
public:
   TestStore() : TestStoreTemplate<ITest, maker_ptr, TestConfig>() {}

   static TestStore& instance() {
     static TestStore store;
     return store;
  }
};

} // namespace VnV

#endif
