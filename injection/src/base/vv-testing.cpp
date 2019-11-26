
/** @file vv-testing.cpp **/

#include "vv-testing.h"
#include "vv-logging.h"

#include <dlfcn.h>

#include <iostream>
#include <sstream>
#include "vv-output.h"
#include "vv-schema.h"
#include "VnV.h"
using namespace VnV;
using nlohmann::json_schema::json_validator;

void* DefaultTransform::_Transform(std::pair<std::string, void*> ip,
                                  std::string testParameterType) {
  if (ip.first.compare(testParameterType) == 0) return ip.second;
  throw "Default Transform does not apply";
}


void ITransform::addInnerTransform(std::string trans) {
      innerTransforms.push_back(trans);
}

void* ITransform::Transform(std::pair<std::string,void*> ip, std::string tp) {
    for (auto it = innerTransforms.rbegin(); it != innerTransforms.rend(); it++) {
       ITransform* itrans = TestStore::getTestStore().getTransform(*it);
       if ( itrans != nullptr) {
        ip.second = itrans->Transform(ip,tp);
       }
    }
    return _Transform(ip,tp);
}

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


void TestConfig::addTransform(std::string testParameter,
                                   std::string ipParameter, std::vector<std::string> trans) {
  transforms.insert(
      std::make_pair(testParameter, std::make_pair(ipParameter, trans)));
}






std::pair<std::string, std::shared_ptr<ITransform>>
TestConfig::getTransform(std::string s) const {

  //VnV_Debug("Looking for a transform called {}.", s);
  auto it = transforms.find(s);
  if (it != transforms.end()) {
    //VnV_Debug("Found a transform called {} {}. ", it->first , it->second.first) ;
    std::shared_ptr<ITransform> transform = nullptr;

    for ( auto &itt : it->second.second ) {
       if ( itt == it->second.second.front() )
            transform.reset(TestStore::getTestStore().getTransform(itt));
       else {
           transform->addInnerTransform(itt);
       }

    }
    return std::make_pair(it->second.first, transform);
  } else {
       //VnV_Debug("Could not find a transform called {}. Returning the default transform", s);
       return std::make_pair(s, std::make_shared<DefaultTransform>());
    }
}

void TestConfig::print() {
    VnV_BeginStage("Test Configuration %s", getName().c_str());
    VnV_Info("Expected Result: %s", getExpectedResult().dump().c_str());
    VnV_Info("Configuration Options: %s", getAdditionalParameters().dump().c_str());
    VnV_BeginStage("Injection Point Mapping");
    for ( auto it : transforms ) {
        std::ostringstream oss;
        int close = 0;
        for ( auto itt : it.second.second) {
            oss << itt << "(";
            close++;
        }
        oss << it.second.first ;
        while (close-- > 0) {
            oss << ")"; close--;
        }
        VnV_Info("%s -> %s ", it.first.c_str(), oss.str().c_str());

    }
    VnV_EndStage("");
    VnV_EndStage("");
}

TestConfig::TestConfig(std::string name, json &testConfigJson) {
  setName(name);
  additionalParameters = testConfigJson["configuration"];
  expectedResult = testConfigJson["expectedResult"];

  for (auto& param : testConfigJson["parameters"].items()) {
       std::string testParameter = param.key();
       std::string injectionParameter;
       std::vector<std::string> transforms;
       // Three options for a parameter are given
       // 1) testPName:ipName -> testP = ipName;
       // 2) testPName:{ parameter:ipName ,transform :tname } -> testP = tname(ipName)
       // 3) testPName:{parameter:ipName, transform: [tname1,tname2,...,tnameN]} -> testP = tname1(tname2(...(tnameN(ipName))...))
       if ( param.value().type() == json::value_t::string) {
           injectionParameter = param.value().get<std::string>();
       } else {
           injectionParameter = param.value().find("parameter")->get<std::string>();
           if ( param.value()["transform"].type() == json::value_t::string ) {
               transforms.push_back(param.value()["transform"].get<std::string>());
           } else {
               for ( auto tform : param.value()["transform"].items()) {
                   transforms.push_back(tform.value().get<std::string>());
               }
           }
       }
       std::cout << testParameter << " " << injectionParameter << std::endl;
       addTransform(testParameter,injectionParameter,transforms);
  }
}

ITransform::ITransform() {}

ITest::ITest(TestConfig &config) : m_config(config)   {
}


// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(IOutputEngine* engine, int stageVal, NTV& params) {
    //VnV_Debug("Runnnig Test {} " , m_config.getName());

    EngineStore::getEngineStore().getEngineManager()->startTest(
        m_config.getName(), 0);
    TestStatus s = runTest(engine, stageVal, params);
    EngineStore::getEngineStore().getEngineManager()->stopTest(
        (s == SUCCESS) ? true : false);
    return s;
}

ITest::~ITest() {}

TestStore::TestStore() {}

TestStore& TestStore::getTestStore() {
  static TestStore* store = new TestStore();
  return *store;
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

    json test_schema;
    auto itt = registeredTests.find(name);
    if (itt == registeredTests.end()) {

      // This is the first time we have encountered this test.
      // So, we need to build the schema for it, and validate.
      json testDeclaration = it->second.second();

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
      registeredTests.insert(std::make_pair(name, test_schema));
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
    std:: cout << test_schema << " \n\n " << testConfigJson << std::endl;

    validator.validate(testConfigJson);

    // Create the Test Config File
    return TestConfig(name, testConfigJson);
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

ITransform* TestStore::getTransform(std::string name) {
  auto it = TestStore::getTestStore().trans_factory.find(name);

  if (it != TestStore::getTestStore().trans_factory.end()) {
    return it->second.first();
  } else if ( name.compare("default")==0) {
      return new DefaultTransform();
  }
  return nullptr;
}

void TestStore::print() {
    VnV_BeginStage("Test Store Configuration");
    VnV_BeginStage("Loaded Libraries");
    for ( auto it : testLibraryPaths) {
        VnV_Info("%s",it.c_str());
    }
    VnV_EndStage("");
    VnV_BeginStage("Loaded Tests");
    for ( auto it : test_factory ) {
        VnV_BeginStage("Test: %s" , it.first.c_str());
        VnV_Info("Schema %s", it.second.second().dump().c_str());
        VnV_EndStage("");
    }

    VnV_BeginStage("Loaded Transforms");
    for ( auto it : trans_factory) {
        VnV_BeginStage("Transform: %s", it.first.c_str());
        VnV_Info("Schema: %s" , it.second.second().dump().c_str());
        VnV_EndStage("");
    }
    VnV_EndStage("");
}

void TestStore::addTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  trans_factory.insert(std::make_pair(name, std::make_pair(t,v)));
}

void VnV_registerTest(std::string name, maker_ptr m, declare_test_ptr v) {
  TestStore::getTestStore().addTest(name, m, v);
}

void VnV_registerTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  TestStore::getTestStore().addTransform(name, t,v);
}
