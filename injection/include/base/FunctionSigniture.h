#ifndef TEMPLATE_CALLBACK_H
#define TEMPLATE_CALLBACK_H

#include <map>
#include <regex>
#include <string>

#include "common-interfaces/prettyfunction.h"
#include "json-schema.hpp"

namespace VnV {
using nlohmann::json;

class FunctionSigniture {
 public:
  struct VnV_Function_Sig fsig;
  FunctionSigniture(struct VnV_Function_Sig sig) : fsig(sig) {}
  bool match(std::string match);
  bool run(json& /* runConfig */) { return true; }
};

}  // namespace VnV
#endif