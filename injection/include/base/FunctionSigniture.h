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
    
    // Match only really cares about template arguments. 
    // We know its the same function -- Just the arguments are different, so 
    // we just need to make sure the parts within [with=....] are the same. 
    
    // If they are identical, then they match :)
    if (s.compare(m) == 0) return true; 
  

    auto a = s.find_last_of("[with");
    auto b = m.find_last_of("[with");
    
     if (a != std::string::npos && b != std::string::npos) {
        if ( s.substr(a).compare(m.substr(b)) == 0 ) {
          return true;
        }   
    }     
  
    std::cout << "Match failed\n" << s << "\n" << m << std::endl;
    return false;

  }
  bool run(json& runConfig) {return true;}
};

}  // namespace VnV
#endif