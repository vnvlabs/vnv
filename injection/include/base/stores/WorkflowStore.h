﻿
/**
  @file ActionStore.h
**/
#ifndef VV_WORKFLOWSTORE_HEADER
#define VV_WORKFLOWSTORE_HEADER

#include <map>
#include <string>

#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/IWorkflow.h"

namespace VnV {

class WorkflowStore : public BaseStore {
  std::map<std::string, std::pair<json, scheduler_ptr>> scheduler_factory;
  std::map<std::string, std::pair<json, validator_ptr>> validator_factory;
  std::map<std::string, std::pair<json, script_gen_ptr>> script_factory;
  std::map<std::string, std::pair<json, workflow_ptr>> workflow_factory;
  

  template <typename T>
  std::shared_ptr<T> getComponent(std::map<std::string, std::pair<json, T* (*)(const json& config)>>& map,
                                  std::string packageName, std::string name, const nlohmann::json& config) ; 

  void loadSubJob(std::string package, std::string name, const json& config, std::shared_ptr<JobManager> jm) {
    jm->pushCreator(package, name);
    getWorkflow(package, name, config)->createJob(*jm);
    jm->popCreator();
  }

 public:
  WorkflowStore() {}



  std::shared_ptr<IValidator> getValidator(std::string packageName, std::string name, const nlohmann::json& config);

  std::shared_ptr<IScheduler> getScheduler(std::string packageName, std::string name, const nlohmann::json& config);

  std::shared_ptr<IScriptGenerator> getScript(std::string packageName, std::string name, const nlohmann::json& config) ;

  std::shared_ptr<IWorkflow> getWorkflow(std::string packageName, std::string name, const nlohmann::json& config) ;


  VnV::Job getJob(std::string packageName, std::string name, std::shared_ptr<VnV::IScheduler> scheduler, const nlohmann::json& config);

  
  std::shared_ptr<JobManager> buildJobManager(std::string mainApplicationName, std::string workflowName,
                                              std::vector<WorkflowConfig>& workflows) {
    
    auto a = std::make_shared<JobManager>(mainApplicationName, "Workflow");
    for (auto it : workflows) {
      loadSubJob(it.package, it.name, it.config, a);
    }
    return a;
  }

  bool registeredWorkflow(std::string package, std::string name) {
    return true;  // We dont know which ones will run.
  }

  void addScheduler(std::string packageName, std::string name, const json& schema, scheduler_ptr m) {
    scheduler_factory[packageName + ":" + name] = std::make_pair(schema, m);
  }

  void addValidator(std::string packageName, std::string name, const json& schema, validator_ptr m) {
    validator_factory[packageName + ":" + name] = std::make_pair(schema, m);
  }

  void addScriptGenerator(std::string packageName, std::string name, const json& schema, script_gen_ptr m) {
    script_factory[packageName + ":" + name] = std::make_pair(schema, m);
  }

  void addWorkflow(std::string packageName, std::string name, const json& schema, workflow_ptr m) {
    workflow_factory[packageName + ":" + name] = std::make_pair(schema, m);
  }

  json schema(json& packageJson) {
    nlohmann::json m = R"({"type":"object"})"_json;
    nlohmann::json properties = json::object();

    for (auto& it : workflow_factory) {
      properties[it.first] = it.second.first;
    }
    m["properties"] = properties;
    m["additionalProperties"] = false;
    return m;
  }

  static WorkflowStore& instance();
};

}  // namespace VnV

#endif
