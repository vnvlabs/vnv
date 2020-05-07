
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

void OutputEngineManager::document(VnV_Comm comm,std::string pname, std::string id, NTV &map) {
    documentationStartedCallBack(comm,pname, id);
    VnVParameterSet parameterSet = DocumentationStore::instance().getParameterMap(pname, id, map);
    for (auto it : parameterSet) {
        if (it.second.getType() == "double") {
            double t = *(it.second.getPtr<double>("double",false));
            Put(comm,it.first,t);
        } else if (it.second.getType() == "int") {
            int t = *(it.second.getPtr<int>("int",false));
            Put(comm,it.first,t);
        } else if (it.second.getType() == "float") {
            float t = *(it.second.getPtr<float>("double",false));
            Put(comm,it.first,t);
        } else if (it.second.getType() == "char*") {
            std::string  t = *(it.second.getPtr<char*>("char*",false));
            Put(comm,it.first,t);
        } else  if (it.second.getType() == "long") {
            long t = *(it.second.getPtr<long>("long",false));
            Put(comm,it.first,t);
        } else if (it.second.getType() == "VnV::CppDocumentaiton::Serializer") {
            // Allow for a custom serializer.
            CppDocumentation::Serialize *ss = it.second.getPtr<CppDocumentation::Serialize>(it.second.getType(),false);
            ISerializer* serialize = SerializerStore::getSerializerStore().getSerializerByName(ss->name);
            if (serialize!=nullptr) {
                std::string s = serialize->Serialize(it.second.getType(),ss->ptr);
                Put(comm,it.first,s);
            }
        }
        else {
            // Check to see if we have a serializer that will work.
            ISerializer* serialize = SerializerStore::getSerializerStore().getSerializerFor(it.second.getType());
            if (serialize!=nullptr) {
                std::string s = serialize->Serialize(it.second.getType(),it.second.getRawPtr());
                Put(comm,it.first,s);
            }
        }
    }
    documentationEndedCallBack(comm,pname,id);
}

IOutputEngine* OutputEngineManager::getOutputEngine() {
    return dynamic_cast<IOutputEngine*>(this);
}

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  OutputEngineStore::getOutputEngineStore().registerEngine(name, r);
}



