#ifndef VNV_ACTION_TYPE_HEADER
#define VNV_ACTION_TYPE_HEADER


#include <string>
namespace VnV {

class ActionType {
  std::string s;
  ActionType(std::string s);
public:
  bool equals(std::string s);
  static ActionType& configure();
  static ActionType& finalize();
};

}

#endif