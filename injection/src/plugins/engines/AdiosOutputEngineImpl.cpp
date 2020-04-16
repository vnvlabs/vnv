
/** @file AdiosOutputEngineImpl.cpp **/

#include "plugins/engines/AdiosOutputEngineImpl.h"
#include "base/exceptions.h"

using namespace VnV;
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


AdiosEngine::AdiosEngine(adios2::Engine& e, adios2::IO& w)
    : writer(w), engine(e)  {
    Define(VariableEnum::String, "LOGS");
}

void AdiosEngine::Put(std::string variableName, const double& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, const int& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, const float& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, const long& value) {
  engine.Put(variableName, value);
}
void AdiosEngine::Put(std::string variableName, const json& value) {
  engine.Put(variableName, value.dump());
}

void AdiosEngine::Put(std::string variableName, const std::string& value) {
  engine.Put(variableName, value);
}


void AdiosEngine::Log(const char* package, int stage, std::string level, std::string message) {
    std::ostringstream oss;
    oss << getIndent(stage) << "[" << package << ":" << level << "]: " << message;
    engine.Put("LOGS", oss.str() );
}

void AdiosEngine::Define(VariableEnum type, std::string name) {
    switch (type) {
        case VariableEnum::Double: writer.DefineVariable<double>(name); break;
        case VariableEnum::Long: writer.DefineVariable<long>(name); break;
        case VariableEnum::Int: writer.DefineVariable<int>(name); break;
        case VariableEnum::Float: writer.DefineVariable<float>(name); break;
        case VariableEnum::String: writer.DefineVariable<std::string>(name); break;
        default: throw VnVExceptionBase("Adios Engine Does not handle Variable Enum Type ") + VariableEnumFactory::toString(type);
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
  stage = bpWriter.DefineVariable<std::string>("stage");
  type = bpWriter.DefineVariable<std::string>("type");
  markdown = bpWriter.DefineVariable<std::string>("markdown");
  result = bpWriter.DefineVariable<int>("result");
  engine = bpWriter.Open(outfile, adios2::Mode::Write);
  adiosEngine = new AdiosEngine(engine, bpWriter);
}

void AdiosWrapper::injectionPointEndedCallBack(std::string id, InjectionPointType type_, std::string stageId) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    std::string ss = InjectionPointTypeUtils::getType(type_,stageId);
    engine.Put(stage, ss);
    std::string s = "EndIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosWrapper::injectionPointStartedCallBack(std::string id, InjectionPointType type_, std::string stageId) {
  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, id);
    std::string ss = InjectionPointTypeUtils::getType(type_,stageId);
    engine.Put(stage, ss);
    std::string s = "StartIP";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosWrapper::documentationStartedCallBack(std::string pname, std::string id) {
  if (engine) {
    engine.BeginStep();
    std::string s = pname + ":" + id;
    engine.Put(identifier, s);
    std::string ss = "StartDoc";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosWrapper::documentationEndedCallBack(std::string pname, std::string id) {
  if (engine) {
    engine.BeginStep();
    std::string s = pname + ":" + id;
    engine.Put(identifier, s);
    std::string ss = "EndDoc";
    engine.Put(type, s);
    engine.EndStep();
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void AdiosWrapper::testStartedCallBack(std::string testName) {

  if (engine) {
    engine.BeginStep();
    engine.Put(identifier, testName);
    std::string test = "StartTest";
    engine.Put(type, test);
  } else {
    throw VnVExceptionBase("Engine not setup correctly");
  }
}

void AdiosWrapper::testFinishedCallBack(bool result_) {
  if (engine) {
    int res = (result_) ? 1 : 0;
    engine.Put(result, res);
    engine.EndStep();
  }
}

void AdiosWrapper::unitTestStartedCallBack(std::string unitTestName) {
    if (engine) {
        engine.BeginStep();
        engine.Put(identifier, unitTestName);
        std::string test = "StartUnitTest";
        engine.Put(type,test);
    } else {
        throw VnVExceptionBase("Engine not setup correctly.");
    }

}

void AdiosWrapper::unitTestFinishedCallBack(std::map<std::string, bool> &results) {
    if ( engine ) {
        int suiteSuccess = 1;
        for ( auto it: results) {
            int res = (it.second) ? 1 : 0;
            engine.Put(identifier,it.first);
            engine.Put(result, res);
            if (suiteSuccess && !it.second)
                suiteSuccess = false;
        }
        engine.Put(result,suiteSuccess);
        engine.EndStep();
    }
}

IOutputEngine* AdiosWrapper::getOutputEngine() { return adiosEngine; }

json AdiosWrapper::getConfigurationSchema() {
    return __adios_input_schema__;
}

OutputEngineManager* AdiosEngineBuilder() { return new AdiosWrapper(); }
