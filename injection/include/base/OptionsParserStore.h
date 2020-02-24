

/**
  @file OptionsParserStore.h
**/

#ifndef VVOPTIONSPARSER_H
#define VVOPTIONSPARSER_H

#include <map>
#include <string>

#include "c-interfaces/CJson.h"

namespace VnV {

class OptionsParserStore {
 private:
  std::map<std::string, std::pair<options_schema_ptr*,options_callback_ptr*>, std::less<std::string>> factory;
  OptionsParserStore();

 public:
  void add(std::string name, options_schema_ptr m, options_callback_ptr v);
  void callBack(std::string name, json info);

  void parse(json info);

  static OptionsParserStore& instance();

};

}  // namespace VnV
#endif // VVOPTIONSPARSER_H
