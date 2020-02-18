
#include <dlfcn.h>
#include <iostream>

#include "json-schema.hpp"
#include "base/Runtime.h"
#include "base/OptionsParserStore.h"
#include "c-interfaces/logging-interface.h"

using namespace VnV;

OptionsParserStore::OptionsParserStore(){
}

void OptionsParserStore::add(std::string name, options_schema_ptr *m, options_callback_ptr *v){
    factory.insert(std::make_pair(name,std::make_pair(m,v)));
}

void OptionsParserStore::callBack(std::string name, json info){

    auto it = factory.find(name);
    if ( it != factory.end() ) {
       json schema = json::parse(it->second.first());
       nlohmann::json_schema::json_validator validator;
       validator.set_root_schema(schema);
       validator.validate(info);
       c_json j = {&info};
       it->second.second(j);
    } else {
        VnV_Warn("Unknown Options Configuration Name %s", name.c_str());
    }
}

OptionsParserStore& OptionsParserStore::instance(){
  static OptionsParserStore store;
  return store;
}

void OptionsParserStore::parse(json info) {

    for ( auto &it : factory ) {
       std::cout << "it: " << it.first << std::endl;
       auto found = info.find(it.first);
       if (found != info.end()) {
          callBack(it.first, found.value());
       } else {
          json foundJson = R"({})"_json;
          callBack(it.first, foundJson);
       }
   }
}


