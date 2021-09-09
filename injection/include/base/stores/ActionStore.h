
/**
  @file ActionStore.h
**/
#ifndef VV_ACTIONSTORE_HEADER
#define VV_ACTIONSTORE_HEADER

#include <map>
#include <string>

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IAction.h"
#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
namespace VnV {


class ActionStore : public BaseStore {
  
  std::map<std::string, std::map<std::string, action_ptr, std::less<std::string>>> action_factory;
 
 public:


  ActionStore();

  void runAction(ICommunicator_ptr comm, std::string packageName,
               std::string Name, const json& config, IAction* tester, ActionType& type);


  void addAction(std::string packageName, std::string name, action_ptr m);

  IAction* getAction(std::string packageName, std::string name);

  void runAction(ICommunicator_ptr comm, std::string packageName,
               std::string testName, const json& config, ActionType& type);


  void runAll(VnV_Comm comm, VnV::ActionInfo info, ActionType& type);

  void print();

  nlohmann::json schema();

  static ActionStore& instance();

};

}  // namespace VnV

#endif
