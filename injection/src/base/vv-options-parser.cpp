
#include "vv-options-parser.h"
#include <dlfcn.h>
#include <iostream>
#include "vv-runtime.h"
#include "json-schema.hpp"
using namespace VnV;

OptionsParserStore::OptionsParserStore(){
}

void OptionsParserStore::add(std::string name, options_schema_ptr *m, options_callback_ptr *v){
    factory.insert(std::make_pair(name,std::make_pair(m,v)));
}

void OptionsParserStore::callBack(std::string name, json info){

    auto it = factory.find(name);
    if ( it != factory.end() ) {
       json schema = it->second.first();
       nlohmann::json_schema::json_validator validator;
       validator.set_root_schema(schema);
       validator.validate(info);
       it->second.second(info);
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
       json schema = it.second.first();
       nlohmann::json_schema::json_validator validator;
       validator.set_root_schema(schema);

        auto found = info.find(it.first);
        json foundJson = R"({})"_json;
        if ( found != info.end() ) {
            foundJson = found.value();
        }
        validator.validate(foundJson);
        it.second.second(foundJson);
   }
}

void VnV_registerOptions(std::string name, VnV::options_schema_ptr s, VnV::options_callback_ptr v) {
   OptionsParserStore::instance().add(name,s,v);
}
