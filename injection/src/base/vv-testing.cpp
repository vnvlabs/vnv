
/** @file vv-testing.cpp **/

#include "vv-testing.h"
#include "vv-logging.h"

#include <dlfcn.h>
#include <link.h>

#include <iostream>

#include "vv-output.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

void* DefaultTransform::Transform(std::pair<std::string, void*> ip,
                                  std::string testParameterType) {
  if (ip.first.compare(testParameterType) == 0) return ip.second;
  throw "Default Transform does not apply";
}



bool TestConfig::runOnStage(int index) {
  VnV_Debug("{} {} {}", getName(), index, stages.size());
    return (stages.find(index) != stages.end());
}

void TestConfig::addTestStage(TestStageConfig config) {
  stages.insert(std::make_pair(config.getInjectionPointStageId(), config));
}

void TestStageConfig::setExpectedResult(json expectedResult) {
  this->expectedResult = expectedResult;
}

json TestStageConfig::getExpectedResult() { return expectedResult; }

void TestConfig::setName(std::string name) { testName = name; }

void TestConfig::addParameter(const std::pair<std::string, std::string>& pair) {
  config_params.insert(pair);
}

std::string TestConfig::getName() { return testName; }

TestStageConfig TestConfig::getStage(int stage) {
  auto it = stages.find(stage);
  if (it != stages.end()) return it->second;
  throw "Stage not found ";
}

void TestStageConfig::addTransform(std::string testParameter,
                                   std::string ipParameter, std::string trans) {
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

void TestConfig::setAdditionalProperties(json additionalProperties) {
  userProperties = additionalProperties;
}
json TestConfig::getAdditionalProperties() { return userProperties; }

std::pair<std::string, std::shared_ptr<ITransform>>
TestStageConfig::getTransform(std::string s) {

  VnV_Debug("Looking for a transform called {}.", s);
  auto it = transforms.find(s);
  if (it != transforms.end()) {
    VnV_Debug("Found a transform called {} {} {}. ", it->first , it->second.first , it->second.second);
    std::shared_ptr<ITransform> transform = nullptr;
    transform.reset(TestStore::getTestStore().getTransform(it->second.second));
    return std::make_pair(it->second.first, transform);
  } else {
       VnV_Debug("Could not find a transform called {}. Returning the default transform", s);
       return std::make_pair(s, std::make_shared<DefaultTransform>());
    }
}

ITransform::ITransform() {}

ITest::ITest() {}

void ITest::set(TestConfig& config) {
  m_config = config;

  // Validate the additional properties passed into the test config
  json schema = getAdditionalPropertiesSchema();
  if (!schema.empty()) {
    json_validator validator;
    validator.set_root_schema(schema);
    validator.validate(config.getAdditionalProperties());
  }

  // Validate the expected results passed in.
  for (auto it : m_config.getStages()) {
    json s = getExpectedResultSchema(it.first);
    if (!s.empty()) {
      json_validator validator;
      validator.set_root_schema(schema);
      validator.validate(it.second.getExpectedResult());
    }
  }

  // Call the users init function to let them know the config
  // is set, and all inputs have been validated.
  init();
}

json ITest::getExpectedResultSchema(int /*stage*/) { return R"({})"_json; }
json ITest::getAdditionalPropertiesSchema() { return R"({})"_json; }

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
                        variable_register_ptr v) {
  test_factory[name] = std::make_pair(m, v);
}

ITest* TestStore::getTest(TestConfig& config) {
  std::string name = config.getName();

  auto it = test_factory.find(name);
  if (it != test_factory.end()) {
    auto itt = registeredTests.find(name);
    if (itt == registeredTests.end()) {
      it->second.second(
          EngineStore::getEngineStore().getEngineManager()->getOutputEngine());
      registeredTests.insert(name);
    }
    ITest * t = it->second.first();
    t->set(config);
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

void VnV_registerTest(std::string name, maker_ptr m, variable_register_ptr v) {
  TestStore::getTestStore().addTest(name, m, v);
}

void VnV_registerTransform(std::string name, trans_ptr t) {
  TestStore::getTestStore().addTransform(name, t);
}


