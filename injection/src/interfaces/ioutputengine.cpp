
#include "interfaces/ioutputengine.h"
#include "json-schema.hpp"
#include "c-interfaces/logging-interface.h"
#include "base/OutputEngineStore.h"
#include "base/SerializerStore.h"
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

void IOutputEngine::Put(std::string /*variableName*/, double& /**value**/){throw "Engine Does not support type double";}
void IOutputEngine::Put(std::string /*variableName*/, int& /**value**/){throw "Engine Does not support type int";}
void IOutputEngine::Put(std::string /*variableName*/, float& /**value**/){throw "Engine Does not support type float";}
void IOutputEngine::Put(std::string /*variableName*/, long& /**value**/){throw "Engine Does not support type long";}
void IOutputEngine::Put(std::string /*variableName*/, std::string& /**value**/){throw "Engine Does not support type string";}
void IOutputEngine::Log(const char *, int, std::string, std::string) { throw "Engine does not support in engine logging";}

void IOutputEngine::Put(std::string variableName, std::string serializer, std::string inputType, void* object) {
    std::string s = SerializerStore::getSerializerStore().getSerializer(serializer)->Serialize(inputType,object);
    Put(variableName,s);
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

void OutputEngineManager::print() {
    VnV_Info("Print not implemented for this Output Engine Manager");
}

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  OutputEngineStore::getOutputEngineStore().registerEngine(name, r);
}

OutputEngineManager::~OutputEngineManager() {}
