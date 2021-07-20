 
 #include "base/ActionType.h"

 using namespace VnV;

 ActionType::ActionType(std::string s) {
    this->s = s;
  }

  bool ActionType::equals(std::string s) {
    return s.compare(this->s) == 0;
  }

  ActionType& ActionType::configure() { 
    static ActionType configAction("configure");
    return configAction;
  }
  
  ActionType& ActionType::finalize() {
    static ActionType finalizeAction("finalize");
    return finalizeAction;  
  }
