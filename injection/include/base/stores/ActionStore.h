
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
namespace VnV {


class ActionStore {
 private:
  std::map<std::string, std::map<std::string, action_ptr*, std::less<std::string>>> action_factory;

  ActionStore();

  void runAction(Communication::ICommunicator_ptr comm, std::string packageName,
               std::string Name, const json& config, IAction* tester, ActionType& type);


 public:
  void addAction(std::string packageName, std::string name, action_ptr m);

  IAction* getAction(std::string packageName, std::string name);

  static ActionStore& getActionStore();

  void runAction(Communication::ICommunicator_ptr comm, std::string packageName,
               std::string testName, const json& config, ActionType& type);


  void runAll(VnV_Comm comm, VnV::ActionInfo info, ActionType& type);

  void print();
};

}  // namespace VnV

#endif
