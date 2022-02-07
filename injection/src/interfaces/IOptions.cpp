/**
  @file CJson.cpp
**/

#include "interfaces/IOptions.h"
#include "base/stores/OptionsParserStore.h"
using nlohmann::json;


void VnV::RegisterOptions(std::string packageName, std::string schema,
                          options_cpp_callback_ptr callback) {
  
  try {
    json j = json::parse(schema);
    VnV::RegisterOptions_Json(packageName, j, callback);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME,"Error Registering Options");  
  }

}

void VnV::RegisterOptions_Json(std::string name, json& schema,
                               options_cpp_callback_ptr callback) {

  try {
    VnV::OptionsParserStore::instance().add(name, schema, callback);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME,"Error Registering Options");  
  }

}
