
/**
  @file ITest.cpp
**/
#include "interfaces/ITest.h"

#include "base/stores/OutputEngineStore.h"
#include "base/stores/TestStore.h"
#include "base/stores/TransformStore.h"
#include "common-interfaces/Logging.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

const json& TestConfig::getAdditionalParameters() const { return testConfigJson["configuration"]; }

const json& ITest::getConfigurationJson() const { return m_config.getAdditionalParameters(); }

void TestConfig::setName(std::string name) { testName = name; }

std::string TestConfig::getName() const { return testName; }
std::string TestConfig::getPackage() const { return package; }

void TestConfig::print() {
  VnV_Info(VNVPACKAGENAME, "Configuration Options: %s", getAdditionalParameters().dump().c_str());
}

TestConfig::TestConfig(std::string package, std::string name, json& testConfigJson) {
  setName(name);
  this->package = package;
  this->testConfigJson = testConfigJson;
}

void TestConfig::setParameterMap(VnVParameterSet& args) { parameters = &args; }

VnVParameterSet& TestConfig::getParameterMap() { return *parameters; }

ITest::ITest(TestConfig& config) : m_config(config) { uuid = uid++; }

// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(ICommunicator_ptr comm, OutputEngineManager* engine, InjectionPointType type,
                           std::string stageId) {
  TestStatus s = FAILURE;
  engine->testStartedCallBack(m_config.getPackage(), m_config.getName(), false, uuid);
  try {
    s = runTest(comm, engine->getOutputEngine(), type, stageId);
  } catch (VnVExceptionBase& e) {
    s = FAILURE;
    VnV_Error(VNVPACKAGENAME, "Exception occured during test %s:%s: %s", m_config.getPackage().c_str(),
              m_config.getName().c_str(), e.what());
  }
  engine->testFinishedCallBack((s == SUCCESS) ? true : false);
  return s;
}

ITest::~ITest() {}

long ITest::uid = 0;

void VnV::registerTest(std::string package, std::string name, std::string schema, maker_ptr m) {
  TestStore::instance().addTest(package, name, schema, m);
}
