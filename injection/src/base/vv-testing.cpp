
/** @file vv-testing.cpp **/


#include <dlfcn.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include "base/vv-output.h"
#include "base/vv-schema.h"
#include "base/vv-testing.h"
#include "base/vv-logging.h"
#include "c-interfaces/logging-interface.h"

#include "json-schema.hpp"
using nlohmann::json_schema::json_validator;

using namespace VnV;

TestStore::TestStore() {}

TestStore& TestStore::getTestStore() {
  static TestStore store;
  return store;
}

void TestStore::addTestLibrary(std::string libraryPath) {
  try {
    void* dllib = dlopen(libraryPath.c_str(), RTLD_NOW);
    if (dllib == nullptr)
      std::cerr << dlerror();
    else {
      testLibraries.push_back(dllib);
      testLibraryPaths.push_back(libraryPath);
    }
  } catch (...) {
    std::cout << "Library not found: " << libraryPath << "\n";
  }
}

void TestStore::addTest(std::string name, maker_ptr m,
                        declare_test_ptr v) {
  test_factory[name] = std::make_pair(m, v);
}




std::vector<TestConfig> TestStore::validateTests(std::vector<json> &configs) {
    std::vector<TestConfig> conf;
    for (auto &it : configs) {
        conf.push_back(validateTest(it));
    }
    return conf;
}

TestConfig TestStore::validateTest(json &testJson) {

  if ( testJson.find("name") == testJson.end() ) {
      // This should be impossible. Input Validation should detect test blocks incorretly specified.
      throw "Test Declaration does not contain Test Name";
  }
  std::string name = testJson["name"].get<std::string>();

  auto it = test_factory.find(name);
  if (it != test_factory.end()) {

    json test_schema, testDeclaration;
    auto itt = registeredTests.find(name);
    if (itt == registeredTests.end()) {

      // This is the first time we have encountered this test.
      // So, we need to build the schema for it, and validate.
      testDeclaration = it->second.second();

      //Validate the testDeclaration itself
      json_validator validator;
      validator.set_root_schema(getTestDelcarationJsonSchema());
      validator.validate(testDeclaration);

      //Define the variables listed in the test.
      IOutputEngine* engine = EngineStore::getEngineStore().getEngineManager()->getOutputEngine();
      for ( auto it : testDeclaration["io-variables"].items()) {
          engine->Define(VariableEnumFactory::fromString(it.value().get<std::string>()),it.key());
      }

      test_schema = getTestValidationSchema(testDeclaration);
      registeredTests.insert(std::make_pair(name, std::make_pair(test_schema,testDeclaration)));
    } else {
      test_schema = itt->second.first;
      testDeclaration = itt->second.second;
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
    if ( itc == testJson.end()) {
        testConfigJson = R"({})";
    } else {
        testConfigJson = *itc;
    }


    if ( testConfigJson.find("configuration") == testConfigJson.end() ) {
        testConfigJson["configuration"] = R"({})"_json;
    }
    if ( testConfigJson.find("expectedResult") == testConfigJson.end() ) {
        testConfigJson["expectedResult"] = R"({})"_json;
    }
    if ( testConfigJson.find("parameters") == testConfigJson.end() ) {
        testConfigJson["parameters"] = R"({})"_json;
    }
    validator.validate(testConfigJson);

    // Create the Test Config File
    return TestConfig(name, testConfigJson,testDeclaration );
  }
  throw "test not found";
}

ITest* TestStore::getTest(TestConfig& config) {
  std::string name = config.getName();

  auto it = test_factory.find(name);
  if (it != test_factory.end()) {
    ITest * t = it->second.first(config);
    return t;
  }
  return nullptr;
}


void TestStore::print() {
    auto a = VnV_BeginStage("Test Store Configuration");

    auto b =VnV_BeginStage("Loaded Libraries");
    for ( auto it : testLibraryPaths) {
        VnV_Info("%s",it.c_str());
    }
    VnV_EndStage(b);

    b = VnV_BeginStage("Loaded Tests");
    for ( auto it : test_factory ) {
        auto c = VnV_BeginStage("Test: %s" , it.first.c_str());
        VnV_Info("Schema %s", it.second.second().dump().c_str());
        VnV_EndStage(c);
    }
    VnV_EndStage(b);


    VnV_EndStage(a);
}


void VnV::registerTest(std::string name, maker_ptr m, declare_test_ptr v) {
  TestStore::getTestStore().addTest(name, m, v);
}

