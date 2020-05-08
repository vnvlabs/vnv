
/**
  @file IOutputEngine.cpp
**/
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"
#include "c-interfaces/Logging.h"
#include "base/OutputEngineStore.h"
#include "base/SerializerStore.h"
#include "base/InjectionPoint.h"
#include "base/exceptions.h"
#include<stdarg.h>

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
        throw VnVExceptionBase(s.c_str());
}

std::string VariableEnumFactory::toString(VariableEnum e) {
        switch (e) {
          case VariableEnum::Double: return "Double";
          case VariableEnum::Long: return "Long";
          case VariableEnum::Float: return "Float";
          case VariableEnum::Int: return "Long";
          case VariableEnum::String: return "String";
        }

        throw VnVExceptionBase("VariableEnumFactory::toString: Unhandled Variable Enum Type");
}


void IOutputEngine::Put(VnV_Comm comm, std::string variableName, std::string serializer, std::string inputType, void* object) {
    auto it = SerializerStore::getSerializerStore().getSerializerByName(serializer);
    if (it != nullptr){
      std::string s =  it->Serialize(inputType,object);
      Put(comm, variableName,s);
    } else {
        VnV_Error("Could not Put variable %s because serializer %s was not found", variableName.c_str(), serializer.c_str());
    }
}

void OutputEngineManager::set(json& inputjson) {
  json schema = getConfigurationSchema();

  if (!schema.empty()) {
    json_validator validator;
    validator.set_root_schema(schema);
    validator.validate(inputjson);
  }
  setFromJson(inputjson);
}

IOutputEngine* OutputEngineManager::getOutputEngine() {
    return dynamic_cast<IOutputEngine*>(this);
}

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  OutputEngineStore::getOutputEngineStore().registerEngine(name, r);
}



