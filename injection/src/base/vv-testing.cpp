
/** @file vv-testing.cpp **/

#include "vv-testing.h"
#include "vv-logging.h"

#include <dlfcn.h>
#include <link.h>

#include <iostream>

#include "vv-output.h"
#include "vv-schema.h"

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

bool TestConfig::runOnStage(int index) const {
  VnV_Debug("{} {} {}", getName(), index, stages.size());
    return (stages.find(index) != stages.end());
}

void TestConfig::addTestStage(TestStageConfig config) {
  stages.insert(std::make_pair(config.getInjectionPointStageId(), config));
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

TestStageConfig TestConfig::getStage(int stage) const  {
  auto it = stages.find(stage);
  if (it != stages.end()) return it->second;
  throw "Stage not found ";
}

void TestStageConfig::addTransform(std::string testParameter,
                                   std::string ipParameter, std::vector<std::string> trans) {
  transforms.insert(
      std::make_pair(testParameter, std::make_pair(ipParameter, trans)));
}

std::map<int, TestStageConfig>& TestConfig::getStages() { return stages; }

int TestStageConfig::getInjectionPointStageId() {
  return injectionPointStageId;
}

int TestStageConfig::getTestStageId() { return testStageId; }

void TestStageConfig::setTestStageId(int id) { testStageId = id; }

void TestStageConfig::setInjectionPointStageId(int id) {
  injectionPointStageId = id;
}

std::pair<std::string, std::shared_ptr<ITransform>>
TestStageConfig::getTransform(std::string s) {

  VnV_Debug("Looking for a transform called {}.", s);
  auto it = transforms.find(s);
  if (it != transforms.end()) {
    VnV_Debug("Found a transform called {} {}. ", it->first , it->second.first) ;
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
       VnV_Debug("Could not find a transform called {}. Returning the default transform", s);
       return std::make_pair(s, std::make_shared<DefaultTransform>());
    }
}

TestConfig::TestConfig(std::string name, json &config) {
  setName(name);
  additionalParameters = config["config"];
  expectedResult = config["expectedResult"];

  for (auto& stage : config["stages"].items()) {
    TestStageConfig config;
    config.setTestStageId(std::stoi(stage.key()));
    config.setInjectionPointStageId(stage.value()["ipId"].get<int>());
    for (auto& param : stage.value().items()) {
        if ( param.key().compare("ipId") == 0 ) {
            config.setInjectionPointStageId(param.value().get<int>());
        } else {
            std::string testParameter = param.key();
            std::string injectionParameter;
            std::vector<std::string> transforms;
            if ( param.value().type() == json::value_t::string) {
                injectionParameter = param.value().get<std::string>();
            } else {
                // Its an object of { "",""} or {"":["","",""]}
                auto it  = param.value().items().begin(); // first element
                injectionParameter = it.key();
                if ( it.value().type() == json::value_t::string) {
                  transforms.push_back(it.value().get<std::string>());
                } else {
                  for ( auto itt: it.value().items()) {
                    transforms.push_back(itt.value().get<std::string>());
                  }
                }
            }
            config.addTransform(testParameter,injectionParameter,transforms);
        }
    }
    addTestStage(config);
  }
}

ITransform::ITransform() {}

ITest::ITest(TestConfig &config) : m_config(config)   {
}

int ITest::getTestStage(int stage) const {
      return m_config.getStage(stage).getTestStageId();
}

// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(IOutputEngine* engine, int stageVal, NTV& params) {
    VnV_Debug("Runnnig Test {} " , m_config.getName());

    if (m_config.runOnStage(stageVal)) {
	
    int testVal = m_config.getStage(stageVal).getTestStageId();
    EngineStore::getEngineStore().getEngineManager()->startTest(
        m_config.getName(), testVal);
    TestStatus s = runTest(engine, stageVal, params);
    EngineStore::getEngineStore().getEngineManager()->stopTest(
        (s == SUCCESS) ? true : false);
    return s;
    } else {

    }
  return NOTRUN;
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

TestConfig TestStore::validateTest(json &config) {

  if ( config.find("name") == config.end() ) {
       throw "Test Declaration does not contain Test Name";
  }
  std::string name = config["name"].get<std::string>();

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
          engine->Define(it.key(),it.value().get<std::string>());
      }

      test_schema = getTestValidationSchema(testDeclaration);
      registeredTests.insert(std::make_pair(name, test_schema));
    } else {
      test_schema = itt->second;
    }

    // Validate the config file.
    json_validator validator;
    validator.set_root_schema(test_schema);
    validator.validate(config);

    // Create the Test Config File
    return TestConfig(name, config);
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
    return it->second();
  } else if ( name.compare("default")==0) {
      return new DefaultTransform();
  }
  return nullptr;
}

void TestStore::addTransform(std::string name, trans_ptr t) {
  trans_factory.insert(std::make_pair(name, t));
}

void VnV_registerTest(std::string name, maker_ptr m, declare_test_ptr v) {
  TestStore::getTestStore().addTest(name, m, v);
}

void VnV_registerTransform(std::string name, trans_ptr t) {
  TestStore::getTestStore().addTransform(name, t);
}


