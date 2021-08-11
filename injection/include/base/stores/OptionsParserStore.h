

/**
  @file OptionsParserStore.h
**/

#ifndef VVOPTIONSPARSER_H
#define VVOPTIONSPARSER_H

#include <map>
#include <string>

#include "json-schema.hpp"
using nlohmann::json;

#include "c-interfaces/CJson.h"
#include "base/stores/BaseStore.h"


namespace VnV {

class OptionsParserStore :public BaseStore {
 friend class Runtime; 
 
 
 private:

  std::map<std::string,
           std::pair<json, std::pair<options_callback_ptr*,
                                     options_cpp_callback_ptr*>>,
           std::less<std::string>>
      factory;


 public:

  OptionsParserStore();


  void add(std::string name, json& m, options_callback_ptr v);
  void add(std::string name, json& m, options_cpp_callback_ptr v);
  void callBack(std::string name, json info);

  void parse(json info, json& cmdline);

  json& getSchema(std::string package);

  static OptionsParserStore& instance();


};

}  // namespace VnV
#endif  // VVOPTIONSPARSER_H
