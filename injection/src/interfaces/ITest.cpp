
/**
  @file ITest.cpp
**/
#include "interfaces/ITest.h"
#include "c-interfaces/Logging.h"
#include "base/OutputEngineStore.h"
#include "base/TransformStore.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

const json& TestConfig::getAdditionalParameters() const {
    return testConfigJson["configuration"];
}

const json& TestConfig::getExpectedResult() const {
    return testConfigJson["expectedResult"];
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
    for ( auto it : parameters ) {
        VnV_Info("(Name, type, rtti) = (%s, %s, %s) ", it.first.c_str(), it.second.getType().c_str(), it.second.getRtti().c_str());
    }
    VnV_EndStage(b);
    VnV_EndStage(a);
}

TestConfig::TestConfig(std::string name, json &testConfigJson, json &testDeclarationJson) {
  setName(name);
  this->testConfigJson = testConfigJson;
  this->testDeclarationJson = testDeclarationJson;
}

void TestConfig::setParameterMap(VnVParameterSet &args) {
    //Create a parameterMap For this one.
    parameters.clear();
    json j = testConfigJson["parameters"];
    for (auto &param : j.items()) {
        std::string testParameter = param.key();
        std::string injectionParam = param.value();
        std::string testParamType = testDeclarationJson["parameters"][testParameter].get<std::string>();
        auto injection = args.find(injectionParam);
        if (injection == args.end()) {
            if (isRequired(testParameter)) {
                throw "Required parameter missing";
            }
        } else {
            std::string s = injection->second.getRtti();

            auto it = transformers.find(testParameter);
            if (it != transformers.end()) {
                parameters.insert(std::make_pair(testParameter, VnVParameter(it->second->Transform(injection->second.getRawPtr(),s),testParamType,s)));
            } else {
                // In a test where "isMappingValidFor..." is called, this shoud never happen.
                std::shared_ptr<Transformer> p = TransformStore::getTransformStore().getTransformer(injection->second.getType(),testParamType);
                parameters.insert(std::make_pair(testParameter, VnVParameter(it->second->Transform(injection->second.getRawPtr(),s),testParamType,s)));
                transformers.insert(std::make_pair(testParameter,std::move(p)));
                VnV_Error("Transform for a test parameter was not pregenerated %s:%s", getName().c_str(), testParameter.c_str());
            }
        }
    }
}


const std::map<std::string,VnVParameter>& TestConfig::getParameterMap() const {
    return parameters;
}


bool TestConfig::isRequired(std::string testParameter) const {
    return testDeclarationJson["requiredParameters"].find(testParameter) != testDeclarationJson["requiredParameters"].end();
}


ITest::ITest(TestConfig &config) : m_config(config)   {
}


bool TestConfig::preLoadParameterSet(std::map<std::string, std::string> &parameters)  {
    // Need to check if we can properly map the test, as declared, to this injection point.
    json j = testConfigJson["parameters"];
    for (auto &param : j.items()) {
        std::string testParameter = param.key();
        std::string injectionParam = param.value();
        std::string testParamType = testDeclarationJson["parameters"][testParameter].get<std::string>();
        bool required = isRequired(testParameter);
        auto injection = parameters.find(injectionParam);
        if (injection == parameters.end()) {
            if (isRequired(testParameter)) {
                return false;
            }
        } else {
                std::shared_ptr<Transformer> p = TransformStore::getTransformStore().getTransformer(injection->second,testParamType);
                if (p == nullptr && required) {
                    return false;
                }
                transformers.insert(std::make_pair(testParameter, std::move(p)));

        }
    }
    return true;
}

// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(VnV_Comm comm, OutputEngineManager* engine, InjectionPointType type, std::string stageId) {
    VnV_Debug_MPI(comm,"Runnnig Test %s " , m_config.getName().c_str());

    OutputEngineStore::getOutputEngineStore().getEngineManager()->testStartedCallBack(comm,m_config.getName());
    TestStatus s = runTest(comm,engine,type,stageId );
    OutputEngineStore::getOutputEngineStore().getEngineManager()->testFinishedCallBack(comm,
        (s == SUCCESS) ? true : false);
    return s;
}

ITest::~ITest() {}
