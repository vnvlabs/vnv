
/** @file vv-adios.cpp **/

#include "vv-adios.h"

#include "vv-output.h"

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


AdiosEngine::AdiosEngine(adios2::Engine& e, adios2::IO& i)
    : writer(i), engine(e) {}

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

void AdiosEngine::DefineDouble(std::string name) {
  writer.DefineVariable<double>(name);
}

void AdiosEngine::DefineFloat(std::string name) {
  writer.DefineVariable<float>(name);
}

void AdiosEngine::DefineInt(std::string name) {
  writer.DefineVariable<int>(name);
}

void AdiosEngine::DefineString(std::string name) {
  writer.DefineVariable<std::string>(name);
}

void AdiosEngine::DefineLong(std::string name) {
  writer.DefineVariable<long>(name);
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
