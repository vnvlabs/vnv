
/**
  @file ITest.cpp
**/
#include "interfaces/ITest.h"

#include "base/OutputEngineStore.h"
#include "base/TransformStore.h"
#include "c-interfaces/Logging.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;

const json& TestConfig::getAdditionalParameters() const {
  return testConfigJson["configuration"]; //JsonUtilities::getOrCreate(testConfigJson,"configuration",JsonUtilities::CreateType::Object);
}
const json& ITest::getConfigurationJson() const {
  return m_config.getAdditionalParameters();
}

void TestConfig::setName(std::string name) { testName = name; }

std::string TestConfig::getName() const { return testName; }
std::string TestConfig::getPackage() const { return package; }

void TestConfig::print() {
  int a = VnV_BeginStage("Test Configuration %s", getName().c_str());
  VnV_Info("Configuration Options: %s",
           getAdditionalParameters().dump().c_str());
  int b = VnV_BeginStage("Injection Point Mapping");
  for (auto it : parameters) {
    VnV_Info("(Name, type, rtti) = (%s, %s, %s) ", it.first.c_str(),
             it.second.getType().c_str(), it.second.getRtti().c_str());
  }
  VnV_EndStage(b);
  VnV_EndStage(a);
}

TestConfig::TestConfig(std::string package, std::string name,
                       json& testConfigJson,
                       std::map<std::string, std::string>& params) {
  setName(name);
  this->package = package;
  this->testConfigJson = testConfigJson;
  this->testParameters = params;
}

void TestConfig::setParameterMap(VnVParameterSet& args) {
  // Create a parameterMap For this one.
  parameters.clear();
  json j = testConfigJson["parameters"];
  for (auto& param : j.items()) {
    std::string testParameter = param.key();
    std::string injectionParam = param.value();
    std::string testParamType = testParameters[testParameter];
    auto injection = args.find(injectionParam);
    if (injection == args.end()) {
      // The parameter specified in args, was not found.
      if (isRequired(testParameter)) {
        throw VnVExceptionBase("Required parameter missing");
      }
    } else {
      std::string s = injection->second.getRtti();

      auto it = transformers.find(testParameter);
      if (it != transformers.end()) {
        parameters.insert(std::make_pair(
            testParameter, VnVParameter(it->second->Transform(
                                            injection->second.getRawPtr(), s),
                                        testParamType, s)));
      } else {
        // In a test where "isMappingValidFor..." is called, this shoud never
        // happen.
        std::shared_ptr<Transformer> p =
            TransformStore::getTransformStore().getTransformer(
                injection->second.getType(), testParamType);
        parameters.insert(std::make_pair(
            testParameter, VnVParameter(it->second->Transform(
                                            injection->second.getRawPtr(), s),
                                        testParamType, s)));
        transformers.insert(std::make_pair(testParameter, std::move(p)));
        VnV_Error("Transform for a test parameter was not pregenerated %s:%s",
                  getName().c_str(), testParameter.c_str());
      }
    }
  }
}

const std::map<std::string, VnVParameter>& TestConfig::getParameterMap() const {
  return parameters;
}

bool TestConfig::isRequired(std::string testParameter) const {
  // TODO Tests could have optional parameters. We should define a way to
  // declare a variable as optional. A typed solution might be the best
  // approach. I.e., test variables that end with _opt, or something like that.
  // For now, all test parameters are required parameters.
  return true;
}

ITest::ITest(TestConfig& config) : m_config(config) {}

// parameters is a map of injectionpoint name to injection point type.
bool TestConfig::preLoadParameterSet(
    std::map<std::string, std::string>& parameters) {
  // Need to check if we can properly map the test, as declared, to this
  // injection point.
  json j = testConfigJson["parameters"];  // maps testParam to injection point param.
  for (auto& param : j.items()) {
    // Get the information about the test parameter
    std::string testParameter = param.key();  // The parameter in the test.


    auto testParameterType = testParameters.find(testParameter);
    if (testParameterType == testParameters.end()) {
      return false;
    }
    bool required = isRequired(testParameter);

    // Get the information about the injection point parameter.
    std::string injectionParam = param.value();  // The parameter in the injection point.
    auto injectionParamType = parameters.find(injectionParam);  //

    std::shared_ptr<Transformer> tran;
    if (injectionParamType == parameters.end() && required) {
      if (parameters.size() == 0) {
        /** This is an unknown injection point (it has parameters maybe, but we
         * dont know them **/
        VnV_Warn(
            "Injection point parameters unknown. Parameters will be passed "
            "without type checking");
        tran = TransformStore::getTransformStore().getTransformer(
            testParameterType->second, testParameterType->second);
      } else {
        std::cout << "Could not find " << injectionParam << std::endl;
        return false;  // required parameter not found.
      }
    } else {
      std::cout << "Loojing for transform " << injectionParamType->second
                << " -> " << testParameterType->second << std::endl;
      tran = TransformStore::getTransformStore().getTransformer(
          StringUtils::squash_copy(injectionParamType->second),
          StringUtils::squash_copy(testParameterType->second));
    }

    if (tran == nullptr && required) {
      return false;
    }
    transformers.insert(std::make_pair(testParameter, std::move(tran)));
  }
  return true;
}

// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(VnV_Comm comm, OutputEngineManager* engine,
                           InjectionPointType type, std::string stageId) {
  VnV_Debug_MPI(comm, "Runnnig Test %s ", m_config.getName().c_str());

  engine->testStartedCallBack(comm, m_config.getName());
  TestStatus s = runTest(comm, engine->getOutputEngine(), type, stageId);
  engine->testFinishedCallBack(comm, (s == SUCCESS) ? true : false);
  return s;
}

ITest::~ITest() {}

/**
 * @brief INJECTION_TEST
 * @param argv
 * @param argc
 * @param config
 *
 * This is the way
 */
INJECTION_TEST(test1, int argv, char** argc, std::string config) {
  engine->Put(comm, "hello", 10.0);
  engine->Put(comm, "argv", get<int>("argv"));
  engine->Put(comm, "argc", *get<char**>("argc"));
  return SUCCESS;
}
