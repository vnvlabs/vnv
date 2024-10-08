﻿
/**
  @file IOutputEngine.cpp
**/
#include "interfaces/IOutputEngine.h"

#include <stdarg.h>

#include "shared/exceptions.h"
#include "base/InjectionPoint.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/all.h"
#include "validate/json-schema.hpp"

using namespace VnV;

std::string VnV::InjectionPointTypeUtils::getType(InjectionPointType type, std::string stageId) {
  if (type == InjectionPointType::Begin) {
    return "Begin";
  } else if (type == InjectionPointType::End) {
    return "End";
  } else if (type == InjectionPointType::Single) {
    return "Single";
  } else if (type == InjectionPointType::Child_Begin) {
    return "CBegin";
  } else if (type == InjectionPointType::Child_End) {
    return "CEnd";
  } else if (type == InjectionPointType::Child_Single) {
    return "CSingle";
  } else {
    return stageId;
  }
}

int VnV::InjectionPointTypeUtils::toC(InjectionPointType type) {
  switch (type) {
  case InjectionPointType::Begin:
    return InjectionPointType_Begin;
  case InjectionPointType::Single:
    return InjectionPointType_Single;
  case InjectionPointType::Iter:
    return InjectionPointType_Iter;
  case InjectionPointType::End:
    return InjectionPointType_End;
  case InjectionPointType::Child_End:
    return InjectionPointType_Child_End;
  case InjectionPointType::Child_Begin:
    return InjectionPointType_Child_Begin;
  case InjectionPointType::Child_Iter:
    return InjectionPointType_Child_Iter;
  case InjectionPointType::Child_Single:
    return InjectionPointType_Child_Single;
  } 
  throw INJECTION_BUG_REPORT_(
      "Someone added an injection point type without implementing an intmap "
      "for it - tisk tisk");
}

VariableEnum VariableEnumFactory::fromString(std::string s) {
  if (s.compare("Double") == 0)
    return VariableEnum::Double;
  else if (s.compare("String") == 0)
    return VariableEnum::Long;
  else if (s.compare("Int") == 0)
    return VariableEnum::Int;
  else if (s.compare("Float") == 0)
    return VariableEnum::Float;
  else if (s.compare("Long") == 0)
    return VariableEnum::Long;

  throw INJECTION_EXCEPTION("From String failed for VariableEnum: %s", s.c_str());
}

std::string VariableEnumFactory::toString(VariableEnum e) {
  switch (e) {
  case VariableEnum::Double:
    return "Double";
  case VariableEnum::Long:
    return "Long";
  case VariableEnum::Float:
    return "Float";
  case VariableEnum::Int:
    return "Long";
  case VariableEnum::String:
    return "String";
  }

  throw INJECTION_BUG_REPORT_("VariableEnumFactory::toString: Unhandled Variable Enum Type");
}

void OutputEngineManager::set(ICommunicator_ptr world, json& inputjson, std::string key) {
  this->key = key;
  setFromJson(world, inputjson);
}

IOutputEngine* OutputEngineManager::getOutputEngine() { return dynamic_cast<IOutputEngine*>(this); }

void VnV::registerEngine(std::string name, engine_register_ptr r, VnV::engine_schema_ptr s) {
  OutputEngineStore::instance().registerEngine(name, r, s);
}
