
/**
  @file IOutputEngine.cpp
**/
#include "interfaces/IOutputEngine.h"

#include <stdarg.h>

#include "base/exceptions.h"
#include "base/points/InjectionPoint.h"
#include "base/stores/OutputEngineStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"

using namespace VnV;
using nlohmann::json_schema::json_validator;

std::string VnV::InjectionPointTypeUtils::getType(InjectionPointType type,
                                                  std::string stageId) {
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
  }
  throw VnVExceptionBase(
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

  s = "VariableEnumFactory::fromString: Unknown Variable Type" + s;
  throw VnVExceptionBase(s.c_str());
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

  throw VnVExceptionBase(
      "VariableEnumFactory::toString: Unhandled Variable Enum Type");
}

void OutputEngineManager::set(ICommunicator_ptr world, json& inputjson,
                              std::string key, bool readMode) {
  this->key = key;

  json schema = getConfigurationSchema(readMode);

  if (!schema.empty()) {
    json_validator validator;
    validator.set_root_schema(schema);
    validator.validate(inputjson);
  }

  setFromJson(world, inputjson, readMode);
}

IOutputEngine* OutputEngineManager::getOutputEngine() {
  return dynamic_cast<IOutputEngine*>(this);
}

void VnV::registerEngine(std::string name, engine_register_ptr r) {
  OutputEngineStore::instance().registerEngine(name, r);
}

void VnV::registerReader(std::string name, engine_reader_ptr r) {
  OutputEngineStore::instance().registerReader(name, r);
}