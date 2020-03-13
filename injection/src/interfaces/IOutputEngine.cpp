
/**
  @file IOutputEngine.cpp
**/
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"
#include "c-interfaces/Logging.h"
#include "base/OutputEngineStore.h"
#include "base/SerializerStore.h"
#include "base/InjectionPoint.h"
#include "base/DocumentationStore.h"
#include "c-interfaces/Documentation.h"

using namespace VnV;
using nlohmann::json_schema::json_validator;


std::string VnV::InjectionPointTypeUtils::getType(InjectionPointType type, std::string stageId) {
    if (type == InjectionPointType::Begin) {
        return "Begin";
    } else if (type == InjectionPointType::End) {
        return "End";
    } else if (type == InjectionPointType::Single) {
        return "Single";
    } else {
        return stageId;
    }
}

VariableEnum VariableEnumFactory::fromString(std::string s) {
        if (s.compare("Double") == 0) return VariableEnum::Double;
        else if (s.compare("String") == 0) return VariableEnum::Long;
        else if (s.compare("Int") == 0) return VariableEnum::Int;
        else if (s.compare("Float") == 0) return VariableEnum::Float;
        else if (s.compare("Long") == 0) return VariableEnum::Long;

        s= "VariableEnumFactory::fromString: Unknown Variable Type" + s;
        throw s.c_str();
}

std::string VariableEnumFactory::toString(VariableEnum e) {
        switch (e) {
          case VariableEnum::Double: return "Double";
          case VariableEnum::Long: return "Long";
          case VariableEnum::Float: return "Float";
          case VariableEnum::Int: return "Long";
          case VariableEnum::String: return "String";
        }

        throw "VariableEnumFactory::toString: Unhandled Variable Enum Type";
}

void IOutputEngine::Put(std::string /*variableName*/, const double& /**value**/){throw "Engine Does not support type double";}
void IOutputEngine::Put(std::string /*variableName*/, const int& /**value**/){throw "Engine Does not support type int";}
void IOutputEngine::Put(std::string /*variableName*/, const float& /**value**/){throw "Engine Does not support type float";}
void IOutputEngine::Put(std::string /*variableName*/, const long& /**value**/){throw "Engine Does not support type long";}
void IOutputEngine::Put(std::string /*variableName*/, const std::string& /**value**/){throw "Engine Does not support type string";}
void IOutputEngine::Put(std::string /*variableName*/, const json& /**value**/){throw "Engine Does not support type json";}
void IOutputEngine::Log(const char *, int, std::string, std::string) { throw "Engine does not support in engine logging";}

#include<stdarg.h>

void IOutputEngine::Put(std::string variableName, std::string serializer, std::string inputType, void* object) {
    auto it = SerializerStore::getSerializerStore().getSerializerByName(serializer);
    if (it != nullptr){
      std::string s =  it->Serialize(inputType,object);
      Put(variableName,s);
    } else {
        VnV_Error("Could not Put variable %s because serializer %s was not found", variableName.c_str(), serializer.c_str());
    }
}


IOutputEngine::~IOutputEngine() {}


std::string IOutputEngine::getIndent(int stage) {
    std::string s = "";
    for ( int i = 0; i < std::max(0,stage); i++) s+= "\t";
    return s;
}

static json __default_configuration_schema__ = R"(

{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "http://rnet-tech.net/vv.schema.json",
  "title": "Basic VV RunTime Schema",
  "description": "Schema for a VnV runtime Configuration file",
  "type": "object"
})"_json;



json OutputEngineManager::getConfigurationSchema() {
    return __default_configuration_schema__;
}

void OutputEngineManager::_set(json& inputjson) {
  json schema = getConfigurationSchema();

  if (!schema.empty()) {
    json_validator validator;
    validator.set_root_schema(schema);
    validator.validate(inputjson);
  }
  set(inputjson);
}

void OutputEngineManager::document(std::string pname, std::string id, NTV &map) {
    documentationStartedCallBack(pname, id);
    VnVParameterSet parameterSet = DocumentationStore::instance().getParameterMap(pname, id, map);
    for (auto it : parameterSet) {
        if (it.second.getType() == "double") {
            double t = *(it.second.getPtr<double>("double",false));
            getOutputEngine()->Put(it.first,t);
        } else if (it.second.getType() == "int") {
            int t = *(it.second.getPtr<int>("int",false));
            getOutputEngine()->Put(it.first,t);
        } else if (it.second.getType() == "float") {
            float t = *(it.second.getPtr<float>("double",false));
            getOutputEngine()->Put(it.first,t);
        } else if (it.second.getType() == "char*") {
            std::string  t = *(it.second.getPtr<char*>("char*",false));
            getOutputEngine()->Put(it.first,t);
        } else  if (it.second.getType() == "long") {
            long t = *(it.second.getPtr<long>("long",false));
            getOutputEngine()->Put(it.first,t);
        } else if (it.second.getType() == "VnV::CppDocumentaiton::Serializer") {
            // Allow for a custom serializer.
            CppDocumentation::Serialize *ss = it.second.getPtr<CppDocumentation::Serialize>(it.second.getType(),false);
            ISerializer* serialize = SerializerStore::getSerializerStore().getSerializerByName(ss->name);
            if (serialize!=nullptr) {
                std::string s = serialize->Serialize(it.second.getType(),ss->ptr);
                getOutputEngine()->Put(it.first,s);
            }
        }
        else {
            // Check to see if we have a serializer that will work.
            ISerializer* serialize = SerializerStore::getSerializerStore().getSerializerFor(it.second.getType());
            if (serialize!=nullptr) {
                std::string s = serialize->Serialize(it.second.getType(),it.second.getRawPtr());
                getOutputEngine()->Put(it.first,s);
            }
        }
    }
    documentationEndedCallBack(pname,id);
}

void OutputEngineManager::documentationStartedCallBack(std::string pname, std::string id) {
    VnV_Info("Starting Documentation For %s:%s", pname.c_str(), id.c_str());
    VnV_Warn("Documentation Started Callback not implemented for the Engine Manager.");
}
void OutputEngineManager::documentationEndedCallBack(std::string pname, std::string id) {
    VnV_Info("Ending Documentation For %s:%s", pname.c_str(), id.c_str());
    VnV_Warn("Documentation Ended Callback not implemented for the Engine Manager.");
}

void OutputEngineManager::print() {
    VnV_Info("Print not implemented for this Output Engine Manager");
}

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  OutputEngineStore::getOutputEngineStore().registerEngine(name, r);
}

OutputEngineManager::~OutputEngineManager() {}
