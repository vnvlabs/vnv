
/** @file OptionsParserStore.cpp Implementation of the OptionsParserStore as
 *defined in base/OptionParserStore.h"
 **/

#include "base/stores/OptionsParserStore.h"

#include <iostream>
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
using namespace VnV;

OptionsParserStore::OptionsParserStore() {}

void OptionsParserStore::add(std::string name, json& schema,
                             options_callback_ptr* v) {
  std::pair<options_callback_ptr*, options_cpp_callback_ptr*> x = {v, nullptr};
  factory[name] = {schema, x};
}
void OptionsParserStore::add(std::string name, json& schema,
                             options_cpp_callback_ptr* v) {
  std::pair<options_callback_ptr*, options_cpp_callback_ptr*> x = {nullptr, v};
  factory[name] = {schema, x};
}

void OptionsParserStore::callBack(std::string name, json info) {
  auto it = factory.find(name);
  if (it != factory.end()) {
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(it->second.first);
    validator.validate(info);

    // Pass it back to the callback -- Pick the C or C++ interface depending.
    if (it->second.second.first != nullptr) {
      c_json j = {&info};
      it->second.second.first(j);
    } else if (it->second.second.second != nullptr) {
      it->second.second.second(info);
    }
  } else {
    VnV_Warn(VNVPACKAGENAME, "Unknown Options Configuration Name %s",
             name.c_str());
  }
}

OptionsParserStore& OptionsParserStore::instance() {
  static OptionsParserStore store;
  return store;
}

void OptionsParserStore::parse(json info, json& cmdline) {
  for (auto& it : factory) {
    json& found = JsonUtilities::getOrCreate(info, it.first,
                                             JsonUtilities::CreateType::Object);
    if (cmdline.contains(it.first)) {
      found["command-line"] = cmdline[it.first];
    }
    callBack(it.first, found);
  }
}

nlohmann::json& OptionsParserStore::getSchema(std::string package) {
  auto it = factory.find(package);
  if (it != factory.end()) {
      return it->second.first;
  }
  throw VnV::VnVExceptionBase("no such package");
}
