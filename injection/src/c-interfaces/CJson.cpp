/**
  @file CJson.cpp
**/

#include "c-interfaces/CJson.h"
#include "base/stores/OptionsParserStore.h"
using nlohmann::json;

void _VnV_registerOptions(const char* name, const char* s,
                          options_callback_ptr v) {
  try {
    json k = json::parse(s);
    VnV::OptionsParserStore::instance().add(name, k, v);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Registering Options");
  }
}

