#include "base/stores/WorkflowStore.h"

#include <iostream>
#include <thread>

#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/stores/OutputEngineStore.h"
#include "inja.hpp"

namespace VnV {

BaseStoreInstance(WorkflowStore)

    void registerJobCreator(std::string packageName, std::string name, const std::string schema, job_ptr r) {
  WorkflowStore::instance().addJobCreator(packageName, name, json::parse(schema), r);
}

void registerValidator(std::string packageName, std::string name, const std::string schema, validator_ptr r) {
  WorkflowStore::instance().addValidator(packageName, name, json::parse(schema), r);
}

void registerScriptGenerator(std::string packageName, std::string name, const std::string schema, script_gen_ptr r) {
  WorkflowStore::instance().addScriptGenerator(packageName, name, json::parse(schema), r);
}

void registerScheduler(std::string packageName, std::string name, const std::string schema, scheduler_ptr r) {
  WorkflowStore::instance().addScheduler(packageName, name, json::parse(schema), r);
}

void JobManager::run(ICommunicator_ptr ptr, bool before) {
  bool complete = false;


 

  if (ptr->Rank() == 0) {
    
    auto selfComm = ptr->self();
    
    if (before) {
      OutputEngineStore::instance().getEngineManager()->workflowStartedCallback(selfComm, package, name, getWorkflowFile());
    }

    auto delay = 30;       // 30 seconds between updates.
    auto stepDelay = 500;  // step delay of 500ms at the end of every progress loop.
    auto start_time = std::chrono::high_resolution_clock::now();

    while (!complete) {
      complete = true;

       if (ptr->Size() > 1 && jobs.size() > 0 ) {
           VnV_Warn(VNVPACKAGENAME_S,
             "Workflows are executed using a single process. All additional processes will hang until the workflow is "
             "finished!%s",
             "");
      }

      for (auto& it : jobs) {
        if (it.second->runBeforeApplication() == before) {
          auto a = it.second->progress();

          if (a.state == JobStatus::State::ERROR) {
            if (it.second->errorKillsWorkflow()) {
              complete = true;
              break;
            }
          } else if (a.state == JobStatus::State::COMPLETE) {
          } else {
            complete = false;
          }
        }
      }
      if (!complete) {
        std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));

        if (delay <
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time)
                .count()) {
          OutputEngineStore::instance().getEngineManager()->workflowUpdatedCallback(selfComm, package, name,
                                                                                    getWorkflowFile());
          start_time = std::chrono::high_resolution_clock::now();
        }
      }
    }

    if (!before) {
      OutputEngineStore::instance().getEngineManager()->workflowEndedCallback(selfComm, package, name, getWorkflowFile());
    }
  }
  // Wait for workflow to finish.
  ptr->Barrier();
}

std::string IScriptGenerator::render(std::string templ, const json& data) {
  try {
    return inja::render(templ, data);
  } catch (std::exception &e) {
    throw INJECTION_EXCEPTION("Template Render failed: %s", e.what());
  }
}

void JobManager::subjob(std::string package, std::string name, const json& config) {
  // TODO Naming Conflicts?
  auto creator = WorkflowStore::instance().getJobCreator(package, name, config);
  creators.push(package + ":" + name);
  creator->createJob(*this);
  creators.pop();
}

std::tuple<std::string, std::string, bool> JobManager::add_node(std::shared_ptr<Job> job,
                                                                std::map<std::string, json>& nodes, json& edges) {
  json j = json::object();

  auto s = job->getStatus();
  j["name"] = job->getName();
  j["state"] = s.state;
  j["status"] = s.result.status;
  j["start"] = job->getStartTime();
  j["end"] = job->getEndTime();
  j["creator"] = job->getCreator();
  j["code"] = job->getVnVJobName();
  j["exitCode"] = s.result.exitCode;
  j["stdo"] = s.result.stdo;
  j["stde"] = s.result.stde;

  std::string jobNode = addnode("Job", j, nodes);
  std::string validNode = addnode("SuccessValidation", job->getName(), nodes);

  if (job->getHasVnVReports()) {
    // Add all the VnV Reports.
    std::string vnvJobDir = job->getVnVJobDir();
    std::string jobName = job->getVnVJobName();
    std::vector<std::string> files = DistUtils::listFilesInDirectory(vnvJobDir);
    std::string prefix = "vnv_" + jobName + "_";
    std::string postfix = ".runInfo";
    for (auto it : files) {
      if (it.substr(0, prefix.size()).compare(prefix) == 0) {
        if (it.substr(it.size() - postfix.size()).compare(postfix) == 0) {
          if (!runInfo.contains(it)) {
            std::ifstream ifs(DistUtils::join({vnvJobDir, it}, 0777, false));
            if (ifs.good()) {
              json rinfo = json::parse(ifs);
              runInfo[it] = rinfo;  // Cache that result.
              add_edge(jobNode, addnode("VnVReport", rinfo, nodes), edges);
            }
          } else {
            add_edge(jobNode, addnode("VnVReport", runInfo[it], nodes), edges);
          }
        }
      }
    }
  }

  auto& sr = job->getSuccessRequirements();
  if (sr.size() > 0) {
    for (auto it : sr) {
      std::string depNode = it->addNodesAndEdges(*job, nodes, edges);
      if (!depNode.empty()) {
        add_edge(jobNode, depNode, edges);
        add_edge(depNode, validNode, edges);
      }
    }
  } else {
    add_edge(jobNode, validNode, edges);
  }

  // Add all the dependencies to other jobs.
  for (auto it : job->getDependencies()) {
    std::string depNode = it->addNodesAndEdges(*job, nodes, edges);
    if (!depNode.empty()) {
      add_edge(depNode, jobNode, edges);
    }
  }

  // Add any listed output files (TODO - Link to VnV Input and output files? )
  for (auto it : job->getOutputFiles()) {
    auto t = add_file_node(it, nodes);
    add_edge(jobNode, t, edges);
  }

  return {jobNode, validNode, job->runBeforeApplication()};
}

Job::Job(std::string n, std::shared_ptr<IScheduler> s, std::string creator, JobManager& manager)
    : name(n), scheduler(s), jcreator(creator), jstore(manager) {
  vnv_workflow_id = StringUtils::random(6);
  enviornment["VNV_WORKFLOW_ID"] = vnv_workflow_id;

}  // namespace VnV

}  // namespace VnV