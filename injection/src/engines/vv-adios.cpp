
/** @file vv-adios.cpp **/

#include "vv-adios.h"

#include "vv-output.h"
#include "vv-logging.h"
using namespace VnV;


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


AdiosEngine::AdiosEngine(adios2::Engine& e, adios2::IO& w)
    : writer(w), engine(e)  {
    Define(VariableEnum::String, "LOGS");
}

void AdiosEngine::Put(std::string variableName, double& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, int& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, float& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, long& value) {
  engine.Put(variableName, value);
}

void AdiosEngine::Put(std::string variableName, std::string& value) {
  engine.Put(variableName, value);
}

void AdiosEngine::Log(const char* package, int stage, LogLevel level, std::string message) {
    std::ostringstream oss;
    oss << getIndent(stage) << "[" << package << ":" << logLevelToString(level) << "]: " << message;
    engine.Put("LOGS", oss.str() );
}

void AdiosEngine::Define(VariableEnum type, std::string name) {
    switch (type) {
        case VariableEnum::Double: writer.DefineVariable<double>(name); break;
        case VariableEnum::Long: writer.DefineVariable<long>(name); break;
        case VariableEnum::Int: writer.DefineVariable<int>(name); break;
        case VariableEnum::Float: writer.DefineVariable<float>(name); break;
        case VariableEnum::String: writer.DefineVariable<std::string>(name); break;
        default: throw "Adios Engine Does not handle Variable Enum Type " + VariableEnumFactory::toString(type);
    }
}


AdiosWrapper::AdiosWrapper() {}

void AdiosWrapper::finalize() {
  if (engine) {
    engine.Close();
  }
}

void AdiosWrapper::set(json& config) {

  
  bool debug = false;
  std::string configFile = "";
  std::string outfile = "./vnv-adios.out";

  if ( config.find("debug") != config.end() )
	debug = config["debug"].get<bool>();
  if ( config.find("outFile") != config.end() ) 
	outfile = config["outFile"].get<std::string>();
  if ( config.find("configFile") != config.end() ) 
	configFile = config["configFile"].get<std::string>();

  if ( configFile.empty()) 
  	adios = new adios2::ADIOS(MPI_COMM_WORLD, debug);
  else 
	adios = new adios2::ADIOS(configFile, MPI_COMM_WORLD, debug);
  
  bpWriter = adios->DeclareIO("BPWriter");
  outputFile = bpWriter.AddTransport("File", {{"Library", "POSIX"}, {"Name", outfile.c_str()}});
  identifier = bpWriter.DefineVariable<std::string>("identifier");
  stage = bpWriter.DefineVariable<int>("stage");
  type = bpWriter.DefineVariable<std::string>("type");
  markdown = bpWriter.DefineVariable<std::string>("markdown");
  result = bpWriter.DefineVariable<int>("result");
  engine = bpWriter.Open(outfile, adios2::Mode::Write);
  adiosEngine = new AdiosEngine(engine, bpWriter);
}

void AdiosWrapper::endInjectionPoint(std::string id, int stageVal) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    engine.Put(stage, stageVal);
    std::string s = "EndIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw "Engine not initialized";
  }
}

void AdiosWrapper::startInjectionPoint(std::string id, int stageVal) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    engine.Put(stage, stageVal);
    std::string s = "StartIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw "Engine not initialized";
  }
}

void AdiosWrapper::startTest(std::string testName, int testStageVal) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, testName);
    engine.Put(stage, testStageVal);
    std::string test = "StartTest";
    engine.Put(type, test);
  } else {
    throw "Engine not setup correctly";
  }
}

void AdiosWrapper::stopTest(bool result_) {
  if (engine) {
    int res = (result_) ? 1 : 0;
    engine.Put(result, res);
    engine.EndStep();
  }
}

IOutputEngine* AdiosWrapper::getOutputEngine() { return adiosEngine; }

json AdiosWrapper::getConfigurationSchema() {
	return __adios_input_schema__;
}

extern "C" {
OutputEngineManager* AdiosEngineBuilder() { return new AdiosWrapper(); }
}

class Adios_engine_proxy {
 public:
  Adios_engine_proxy() { VnV_registerEngine("adios", AdiosEngineBuilder); }
};

Adios_engine_proxy adios_engine_proxy;
