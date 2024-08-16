
/** @file OptionsParserStore.cpp Implementation of the OptionsParserStore as
 *defined in base/OptionParserStore.h"
 **/

#include "base/stores/OptionsParserStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"
#include "base/stores/OutputEngineStore.h"
#include "common-interfaces/all.h"

using namespace VnV;

OptionsParserStore::OptionsParserStore() {}

void OptionsParserStore::add(std::string name, json& schema, options_callback_ptr v) {
  std::pair<options_callback_ptr, options_cpp_callback_ptr> x = {v, nullptr};
  factory[name] = {schema, x};
}

void OptionsParserStore::add(std::string name, json& schema, options_cpp_callback_ptr v) {
  std::pair<options_callback_ptr, options_cpp_callback_ptr> x = {nullptr, v};
  factory[name] = {schema, x};
}

nlohmann::json OptionsParserStore::schema(json& packageJson) {
  json p = json::object();
  for (auto& it : factory) {
    p[it.first] = it.second.first;
    p[it.first]["description"] = packageJson[it.first]["Options"]["description"];
  }
  json j = json::object();
  j["type"] = "object";
  j["properties"] = p;
  j["additionalProperties"] = false;
  return j;
}

  void* OptionsParserStore::getResult(std::string name) {
    if (optionResult.find(name) != optionResult.end()) {
      return optionResult[name];
    }
    return nullptr;
  }

void OptionsParserStore::callBack(std::string name, json info, std::vector<std::string>& cmdline, ICommunicator_ptr world) {

  
  auto it = factory.find(name);
  

  
  if (it != factory.end()) {
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(it->second.first);
    validator.validate(info);


    // Pass it back to the callback -- Pick the C or C++ interface depending.
    OutputEngineManager* engine = OutputEngineStore::instance().getEngineManager();


    engine->packageOptionsStartedCallBack(world, name);

    if (it->second.second.first != nullptr) {
      cjson j = {&info};
      optionResult[name] = (*it->second.second.first)(j);
    } else if (it->second.second.second != nullptr) {
      optionResult[name] = (*it->second.second.second)(info, cmdline, engine->getOutputEngine(), world);
    } else {
      optionResult[name] = new RawJsonObject(info);
    }
    engine->packageOptionsEndedCallBack(name);
  } else {
    VnV_Warn(VNVPACKAGENAME, "Unknown Options Configuration Name %s", name.c_str());
  }
}

void OptionsParserStore::parse(json info, std::vector<std::string>& cmdline, ICommunicator_ptr world) {

  for (auto& it : factory) {   
    callBack(it.first, info,  cmdline, world);
  }
}

nlohmann::json& OptionsParserStore::getSchema(std::string package) {
  auto it = factory.find(package);
  if (it != factory.end()) {
    return it->second.first;
  }
  throw INJECTION_EXCEPTION("Options Store: No pacakge %s exists", package.c_str());
}

BaseStoreInstance(OptionsParserStore)
