
/** @file AdiosOutputEngineImpl.cpp **/
#include "plugins/engines/adios/AdiosEngineManager.h"

using nlohmann::json;

static json __adios_input_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv.schema.json",
    "title": "Adios Engine Input Schema",
    "description": "Schema for the adios Engine",
    "type": "object",
    "properties": {
        "debug": {
            "type": "boolean"
        },
        "configFile": {
            "type": "string"
        },
        "outFile": {
            "type": "string"
        }
    },
    "additionalProperties": false
})"_json;

INJECTION_ENGINE(adios) {
  return new VnV::PACKAGENAME::Engines::AdiosEngineManager();
}

namespace VnV {
namespace PACKAGENAME {
namespace Engines {

OutputEngineManager* AdiosEngineBuilder() { return new AdiosEngineManager(); }

AdiosEngineManager::AdiosEngineManager() {}

AdiosEngineManager::~AdiosEngineManager() {}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const double& value) {
  engine.Put(variableName, value);
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const int& value) {
  engine.Put(variableName, value);
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const float& value) {
  engine.Put(variableName, value);
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const long& value) {
  engine.Put(variableName, value);
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const bool& value) {
  const int val = value;
  engine.Put(variableName, val);
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const nlohmann::json& value) {
  engine.Put(variableName, value.dump());
}

void AdiosEngineManager::Put(VnV_Comm, std::string variableName,
                             const std::string& value) {
  engine.Put(variableName, value);
}

void AdiosEngineManager::Log(VnV_Comm, const char* package, int stage,
                             std::string level, std::string message) {
  std::ostringstream oss;
  oss << "[" << package << ":" << level << "]: " << message;
  engine.Put("LOGS", oss.str());
}

void AdiosEngineManager::finalize() {
  if (engine) {
    engine.Close();
  }
}

void AdiosEngineManager::setFromJson(nlohmann::json& config) {
  bool debug = false;
  std::string configFile = "";
  std::string outfile = "./vnv-adios.out";

  if (config.find("debug") != config.end()) debug = config["debug"].get<bool>();
  if (config.find("outFile") != config.end())
    outfile = config["outFile"].get<std::string>();
  if (config.find("configFile") != config.end())
    configFile = config["configFile"].get<std::string>();

  if (configFile.empty())
    adios = new adios2::ADIOS(MPI_COMM_WORLD, debug);
  else
    adios = new adios2::ADIOS(configFile, MPI_COMM_WORLD, debug);

  bpWriter = adios->DeclareIO("BPWriter");
  outputFile = bpWriter.AddTransport(
      "File", {{"Library", "POSIX"}, {"Name", outfile.c_str()}});
  identifier = bpWriter.DefineVariable<std::string>("identifier");
  stage = bpWriter.DefineVariable<std::string>("stage");
  type = bpWriter.DefineVariable<std::string>("type");
  markdown = bpWriter.DefineVariable<std::string>("markdown");
  result = bpWriter.DefineVariable<int>("result");
  engine = bpWriter.Open(outfile, adios2::Mode::Write);
}

void AdiosEngineManager::injectionPointEndedCallBack(VnV_Comm, std::string id,
                                                     InjectionPointType type_,
                                                     std::string stageId) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    std::string ss = InjectionPointTypeUtils::getType(type_, stageId);
    engine.Put(stage, ss);
    std::string s = "EndIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosEngineManager::injectionPointStartedCallBack(VnV_Comm, std::string id,
                                                       InjectionPointType type_,
                                                       std::string stageId) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    std::string ss = InjectionPointTypeUtils::getType(type_, stageId);
    engine.Put(stage, ss);
    std::string s = "StartIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosEngineManager::testStartedCallBack(VnV_Comm, std::string testName) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, testName);
    std::string test = "StartTest";
    engine.Put(type, test);
  } else {
    throw VnVExceptionBase("Engine not setup correctly");
  }
}

void AdiosEngineManager::testFinishedCallBack(VnV_Comm, bool result_) {
  if (engine) {
    int res = (result_) ? 1 : 0;
    engine.Put(result, res);
    engine.EndStep();
  }
}

void AdiosEngineManager::unitTestStartedCallBack(VnV_Comm,
                                                 std::string unitTestName) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, unitTestName);
    std::string test = "StartUnitTest";
    engine.Put(type, test);
  } else {
    throw VnVExceptionBase("Engine not setup correctly.");
  }
}

void AdiosEngineManager::unitTestFinishedCallBack(VnV_Comm, IUnitTest* tester) {
  if (engine) {
    int suiteSuccess = 1;
    for (auto it : tester->getResults()) {
      int res = (std::get<2>(it)) ? 1 : 0;
      engine.Put(identifier, std::get<0>(it));
      engine.Put(result, res);
      if (suiteSuccess && !std::get<2>(it)) suiteSuccess = false;
    }
    engine.Put(result, suiteSuccess);
    engine.EndStep();
  }
}
void AdiosEngineManager::dataTypeStartedCallBack(VnV_Comm,
                                                 std::string variableName, std::string dtype) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, variableName);
    std::string test = "StartDataType";
    engine.Put(type, test);
  } else {
    throw VnVExceptionBase("Engine not setup correctly.");
  }
}

void AdiosEngineManager::dataTypeEndedCallBack(VnV_Comm, std::string variableName) {
  if (engine) {
    engine.EndStep();
  }
}
nlohmann::json AdiosEngineManager::getConfigurationSchema() {
  return __adios_input_schema__;
}

std::string AdiosEngineManager::print() { return "Adios Engine Manager"; }

Nodes::IRootNode* AdiosEngineManager::readFromFile(std::string) {
  throw VnVExceptionBase("Reader Not Implemented for Adios Engine");
}

}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
