#include "plugins/engines/jsonoutputengine.h"
#include "c-interfaces/Logging.h"
#include <iostream>
#include <fstream>
#include "base/exceptions.h"

using namespace VnV;

static json __json_engine_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv-debug-schema.json",
    "title": "Json Engine Input Schema",
    "description": "Schema for the debug engine",
    "type": "object",
    "parameters" : {
       "outputFile" : {"type":"string"}
     }
}
)"_json;


JsonEngine::JsonEngine() {
  mainJson = json::object();
  mainJson["info"] = json::object();
  mainJson["children"] = json::array();
  ptr = json::json_pointer("/children");
}

#define LTypes X(double) X(int) X(float) X(long) X(std::string) X(json)
#define X(type) \
void JsonEngine::Put(VnV_Comm comm,std::string variableName, const type& value) {\
    json j;\
    j["name"] = variableName;\
    j["type"] = #type;\
    j["value"] = value;\
    j["Node"] = "Data";\
    append(j);\
}
LTypes
#undef X
#undef LTypes

void JsonEngine::append(json& jsonOb) {
   mainJson.at(ptr).push_back(jsonOb);
}
void JsonEngine::pop(int num) {
    while (num-- > 0 )
        ptr = ptr.parent_pointer();
}

void JsonEngine::push(json &jsonOb) {
    append(jsonOb);
    ptr /= mainJson.at(ptr).size()-1;
}

void JsonEngine::push(json &jsonOb, json::json_pointer ptr) {
    push(jsonOb);
    this->ptr /= ptr;
}

void JsonEngine::Log(VnV_Comm comm,const char *package, int stage,  std::string level, std::string message) {
    json log = json::object();
    log["package"] = package;
    log["stage"] = stage;
    log["level"] = level;
    log["message"] = message;
    append(log);
}

std::string JsonEngine::Dump(int d) {
    return  mainJson.dump(d);
}

void JsonEngine::Define(VariableEnum /*type*/, std::string /*variableName*/) {

}

JsonEngineWrapper::JsonEngineWrapper() {}

json JsonEngineWrapper::getConfigurationSchema() {
    return __json_engine_schema__;
}

void JsonEngineWrapper::finalize() {
  if (!outputFile.empty()) {
      std::ofstream f;
      f.open(outputFile);
      f << jsonEngine->Dump(3);
  } else {
     std::cout << jsonEngine->Dump(3);
  }
}

void JsonEngineWrapper::set(json& config) {
    if (config.contains("outputFile")) {
        this->outputFile = config["outputFile"].get<std::string>();
    }
    jsonEngine = new JsonEngine();
}

void JsonEngineWrapper::injectionPointEndedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageVal) {
  if (jsonEngine) {
      if (type == InjectionPointType::End) {
          jsonEngine->pop(4);
      } else {
          jsonEngine->pop(2);
      }
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::injectionPointStartedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageVal) {
  if (jsonEngine) {
    json ip;

    json stage;
    stage["id"] = id;
    stage["type"] = InjectionPointTypeUtils::getType(type,stageVal);
    stage["stageId"] = stageVal;
    stage["children"] = json::array();

    if (type == InjectionPointType::Begin) {
        json j;
        j["Node"] = "LoopedInjectionPoint";
        j["id"] = id;
        j["stages"] = json::array();
        j["stages"].push_back(stage);
        jsonEngine->push(j,json::json_pointer("/stages/0/children"));
    } else {
        jsonEngine->push(stage,json::json_pointer("/children"));
    }
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::testStartedCallBack(VnV_Comm comm,std::string testName) {
  if (jsonEngine) {
    json j;
    j["Name"] = testName;
    j["Node"] = "Test";
    j["children"] = json::array();
    jsonEngine->push(j,json::json_pointer("/children"));
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::testFinishedCallBack(VnV_Comm comm,bool result_) {
  if (jsonEngine) {
    jsonEngine->pop(2);
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::documentationStartedCallBack(VnV_Comm comm,std::string pname, std::string id) {
  if (jsonEngine) {
      json j;
      j["Node"] = "Docs";
      j["Name"] = id;
      j["Package"] = pname;
      j["children"] = json::array();
      jsonEngine->push(j,json::json_pointer("/children"));
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::documentationEndedCallBack(VnV_Comm comm,std::string pname, std::string id) {
  if (jsonEngine) {
    jsonEngine->pop(2);
  } else {
    throw VnVExceptionBase("Engine not initialized");
  }
}

void JsonEngineWrapper::unitTestStartedCallBack(VnV_Comm comm,std::string unitTestName) {
    if  (jsonEngine) {
        json j;
        j["Node"] = "UnitTest";
        j["Name"] = unitTestName;
        j["children"] = json::array();
        jsonEngine->push(j,json::json_pointer("/children"));
    } else {
        throw VnVExceptionBase("Engine not initialized");
    }
}

void JsonEngineWrapper::unitTestFinishedCallBack(VnV_Comm comm,std::map<std::string, bool> &results){
    if (jsonEngine) {
        json j;
        j["Node"]  = "UnitTestResults";
        for (auto it : results) {
            j[it.first] = it.second;
        }
        jsonEngine->append(j);
        jsonEngine->pop(2);
    } else {
        throw VnVExceptionBase("Engine Not Initialized");
    }
}

IOutputEngine* JsonEngineWrapper::getOutputEngine() { return jsonEngine;}

OutputEngineManager* JsonEngineBuilder() { return new JsonEngineWrapper(); }
