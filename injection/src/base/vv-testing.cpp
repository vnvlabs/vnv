
/** @file vv-testing.cpp **/

#include "vv-testing.h"
#include "vv-logging.h"

#include <dlfcn.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>
#include <sstream>
#include "vv-output.h"
#include "vv-schema.h"
#include "VnV.h"


using namespace VnV;
using nlohmann::json_schema::json_validator;

void* ITransform::Transform(std::string outputType, std::string inputType, void* ptr ) {
    if ( inputType.compare(outputType) == 0 ) {
        return ptr;
    } else {
        throw "Input Type Does not match output type when using Default Transform.";
    }
}

ITransform::ITransform(){};
ITransform::~ITransform(){};

const json& TestConfig::getAdditionalParameters() const {
    return additionalParameters;
}

const json& TestConfig::getExpectedResult() const {
    return expectedResult;
}

const json& ITest::getExpectedResultJson() const {
    return m_config.getExpectedResult();
}
const json& ITest::getConfigurationJson() const {
    return m_config.getAdditionalParameters();
}


void TestConfig::setName(std::string name) { testName = name; }

std::string TestConfig::getName() const { return testName; }





void TestConfig::print() {
    int a = VnV_BeginStage("Test Configuration %s", getName().c_str());
    VnV_Info("Expected Result: %s", getExpectedResult().dump().c_str());
    VnV_Info("Configuration Options: %s", getAdditionalParameters().dump().c_str());
    int b = VnV_BeginStage("Injection Point Mapping");
    for ( auto it : parameterMap ) {
        VnV_Info("%s(%s) -> %s [%d]", it.first.c_str(), it.second.parameterType.c_str(), it.second.injectionPointParameter.c_str(), it.second.required);
    }
    VnV_EndStage(b);
    VnV_EndStage(a);
}

TestConfig::TestConfig(std::string name, json &testConfigJson, json &testDeclarationJson) {
  setName(name);
  additionalParameters = testConfigJson["configuration"];
  expectedResult = testConfigJson["expectedResult"];

  for (auto& param : testConfigJson["parameters"].items()) {
       std::string testParameter = param.key();
       std::string injectionParameter = param.value();
       std::string testParameterType = testDeclarationJson["parameters"][testParameter].get<std::string>();
       bool required = (testDeclarationJson["requiredParameters"].find(testParameter) != testDeclarationJson["requiredParameters"].end());
       parameterMap[testParameter] = { injectionParameter, required, testParameterType };
  }
}


bool TestConfig::isRequired(std::string name) const {
    auto it = parameterMap.find(name);
    if (it != parameterMap.end()) {
        return it->second.required;
    }
    return false;
}


ITest::ITest(TestConfig &config) : m_config(config)   {
}

ParameterMapping TestConfig::getMapping(std::string parmaeterName) const {
    auto it = parameterMap.find(parmaeterName);
    if (it != parameterMap.end()) {
        return it->second;
    }
    throw "Parameter Not Found";
}

void* TestConfig::mapParameter(std::string parameterName, NTV &parameters) const {
  ParameterMapping mapping = getMapping(parameterName);

  //Pull the associated Injection point param from the NTV.
  auto ip_parameter = parameters.find(mapping.injectionPointParameter);
  if (ip_parameter == parameters.end()) {
      if ( mapping.required) {
          throw "Required Parameter was not found";
      } else {
        return nullptr;
      }
  }
  // Transform the injection point parameter into the test parameter
  return TestStore::getTestStore().getTransform(mapping.parameterType, ip_parameter->second.first, ip_parameter->second.second);
}

std::map<std::string, void*> TestConfig::mapParameters(NTV &parameters) const {
    std::map<std::string, void*> rparameters;
    for (auto it: parameterMap) {
        rparameters.insert(std::make_pair(it.first,mapParameter(it.first,parameters)));
    }
    return rparameters;
}

// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(IOutputEngine* engine, InjectionPointType type, std::string stageId, NTV &parameters) {
    VnV_Debug("Runnnig Test %s " , m_config.getName().c_str());

    EngineStore::getEngineStore().getEngineManager()->testStartedCallBack(m_config.getName());
    std::map<std::string,void*> p = m_config.mapParameters(parameters);
    TestStatus s = runTest(engine,type,stageId, p);
    EngineStore::getEngineStore().getEngineManager()->testFinishedCallBack(
        (s == SUCCESS) ? true : false);
    return s;
}

ITest::~ITest() {}

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

void* TestStore::getTransform(std::string from, std::string to, void* ptr) {

   // TODO -- Allow for multiple transforms. Currently, to obtain a transform, we
   // search the trans_map for an entry related to the class "from". If one is
   // found, we search the inner map from an entry named "to". If that is found
   // then the transform with the name trans_map[from][to] has declared that it
   // can do the required transform.

   // To improve this, we could create a directed transform graph. This would allow
   // us to chain together multiple transforms to achieve the desired result.

   // TODO Need to figure out the memeory management for these transforms.

   if  (from.compare(to) == 0) {
       return ptr;
   } else {
     auto t = trans_map.find(from);
     if (t != trans_map.end()) {
       auto tt = t->second.find(to);
       if ( tt != t->second.end()) {
          ITransform *trans = trans_factory.find(tt->second)->second();
          return trans->Transform(to,from,ptr);
       }
     }
     VnV_Warn("Cannot Convert from Object from (%s) to (%s).", from.c_str(),to.c_str());
     return ptr;
   }
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

    b = VnV_BeginStage("Loaded Transforms");
    for ( auto it : trans_factory) {
        VnV_Info("Transform: %s", it.first.c_str());
    }
    VnV_EndStage(b);

    VnV_EndStage(a);
}

void TestStore::addTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  trans_factory.insert(std::make_pair(name,t));
  json x = v();
  json_validator validator;
  validator.set_root_schema(getTransformDeclarationSchema());
  validator.validate(x);

  for( auto from : x.items() ) {
      std::string fromStr = from.key();
      auto from_trans = trans_map.insert(std::make_pair(fromStr,std::map<std::string,std::string>())).first->second;
      for ( auto to : from.value().items() ){
          from_trans.insert(std::make_pair(to.value(),name));
      }
  }
}

void VnV_registerTest(std::string name, maker_ptr m, declare_test_ptr v) {
  TestStore::getTestStore().addTest(name, m, v);
}

void VnV_registerTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  TestStore::getTestStore().addTransform(name, t,v);
}
