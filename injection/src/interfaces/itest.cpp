
#include "interfaces/itest.h"
#include "c-interfaces/logging-interface.h"
#include "base/OutputEngineStore.h"
#include "base/TransformStore.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

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
  return TransformStore::getTransformStore().getTransform(mapping.parameterType, ip_parameter->second.first, ip_parameter->second.second);
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

    OutputEngineStore::getOutputEngineStore().getEngineManager()->testStartedCallBack(m_config.getName());
    std::map<std::string,void*> p = m_config.mapParameters(parameters);
    TestStatus s = runTest(engine,type,stageId, p);
    OutputEngineStore::getOutputEngineStore().getEngineManager()->testFinishedCallBack(
        (s == SUCCESS) ? true : false);
    return s;
}

ITest::~ITest() {}
