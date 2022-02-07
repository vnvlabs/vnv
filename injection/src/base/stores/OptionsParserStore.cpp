
/** @file OptionsParserStore.cpp Implementation of the OptionsParserStore as
 *defined in base/OptionParserStore.h"
 **/

#include "base/stores/OptionsParserStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/stores/OutputEngineStore.h"
#include "c-interfaces/Logging.h"

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

nlohmann::json OptionsParserStore::schema() {
  json p = json::object();
  for (auto& it : factory) {
    p[it.first] = it.second.first;
  }
  json j = json::object();
  j["type"] = "object";
  j["properties"] = p;
  j["additionalProperties"] = false;
  return j;
}

void OptionsParserStore::callBack(std::string name, json info, ICommunicator_ptr world) {
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
      optionResult[name] = (*it->second.second.second)(info, engine->getOutputEngine(), world);
    } else {
      // It was a "raw" options. so -- We copy j and store it
      // objects[pcakge] = jj (or something)
      // MERGE ON MONDAY -- I Updated the options parser so we can return an object. I just
      // forgot to push it i guess.
    }

    engine->packageOptionsEndedCallBack(name);

  } else {
    VnV_Warn(VNVPACKAGENAME, "Unknown Options Configuration Name %s", name.c_str());
  }
}

void OptionsParserStore::parse(json info, json& cmdline, ICommunicator_ptr world) {
  for (auto& it : factory) {
    json& found = JsonUtilities::getOrCreate(info, it.first, JsonUtilities::CreateType::Object);
    if (cmdline.contains(it.first)) {
      found["command-line"] = cmdline[it.first];
    }
    callBack(it.first, found, world);
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
