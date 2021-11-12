#ifndef TEMPLATE_CALLBACK_H
#define TEMPLATE_CALLBACK_H

#include <map>
#include <regex>
#include <string>
#include "base/Utilities.h"
#include "json-schema.hpp"

namespace VnV {
using nlohmann::json;


class FunctionSigniture {
 public:
  struct VnV_Function_Sig fsig;
  FunctionSigniture(struct VnV_Function_Sig sig) : fsig(sig) {}
  bool match(std::string match) { 
    std::string s = StringUtils::squash_copy(fsig.signiture);
    std::string m = StringUtils::squash_copy(match);  
    return s.compare(m) == 0;
  }
  bool run(json& runConfig) {return true;}
};

}  // namespace VnV
#endif