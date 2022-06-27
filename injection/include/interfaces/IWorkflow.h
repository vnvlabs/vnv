#ifndef IWORKFLOW_HEADER
#define IWORKFLOW_HEADER

#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common-interfaces/PackageName.h"
#include "interfaces/ICommunicator.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

class ProcessResult {
 public:
  enum class Status { INITIALIZED, RUNNING, SUCCESS, FAIL, CANCEL, ERROR };

  Status status = Status::INITIALIZED;
  long exitCode = -1;
  std::string stdo = "";
  std::string stde = "";
  ProcessResult(){};
  ProcessResult(Status stat, long e, std::string o, std::string err) : status(stat), exitCode(e), stdo(o), stde(err) {}
};

class JobStatus {
 public:
  enum class State {
    ERROR,
    CANCELLED,
    WAITING_FOR_LAUNCH,
    CHECKING_DEPENDENCIES,
    STAGING_DATA,
    LAUNCHING_JOB,
    RUNNING,
    CHECKING_SUCCESS_REQUIREMENTS,
    JOB_FINISHED,
    COMPLETE,
  };

  std::string jobId = "__";
  State state = State::WAITING_FOR_LAUNCH;
  ProcessResult result;

  JobStatus(){};
  JobStatus(std::string jobId) { this->jobId = jobId; }
};

class IScheduler {
 public:
  enum class CopyType { COPY, MOVE, HARD_LINK, SOFT_LINK, TEXT, ASIS };

  virtual std::string run(std::string script, std::string workingDirectory,
                          const std::map<std::string, std::string>& environ, std::string setup, std::string teardown,
                          int timeout) = 0;

  virtual std::string stage(std::string source, std::string dest, CopyType type, std::string working_directory) = 0;

  virtual void cancel(std::string jobId) = 0;

  virtual ProcessResult status(std::string jobId, long timeout = -1) = 0;

  virtual bool exists(std::string filename, std::string working_directory) = 0;

  virtual ~IScheduler() {}
};

class Job;

class IValidator {
 public:
  enum class Status { VALID, INVALID, WAIT };

  IValidator() {}
  virtual Status validate(const Job& /* job */) { return Status::VALID; };
  virtual std::string addNodesAndEdges(const Job& /* job */, std::map<std::string, nlohmann::json>& /* nodes */,
                                       nlohmann::json& /* edges */) {
    return "";
  }
  virtual void cancel(const Job& /* job */){};
  virtual ~IValidator() {}
};

class JobManager;

class Job {
  friend class JobManager;

  // Required stuff
  std::string name;
  std::shared_ptr<IScheduler> scheduler = nullptr;

  // Optional Stuff
  std::string working_directory = "", setup = "", run = "", teardown = "";
  std::map<std::string, std::string> enviornment;
  std::vector<std::shared_ptr<IValidator>> success_requirements, dependencies;
  std::vector<std::string> outputfiles;
  long start_time = -1, end_time = -1;
  bool killOnError = false;
  bool hasVnVReports = true;

  // How long do you think this will take. Schedulers can use this to
  // set the polling interval for different jobs. I.e., if you think
  // a job is going to take 30 minutes, we might only poll for status
  // updates every 10 minutes to start -> then go to every minute
  // as we get closer.
  long expectedRunTime = 300;  // five mintues.

  std::string jobId = "__";
  std::string jcreator = "";
  JobStatus status;
  std::vector<std::string> errors;
  std::string vnv_workflow_id;
  std::string vnvJobDir = "/tmp";

  bool beforeApplication = true;

  JobManager& jstore;

  Job(std::string n, std::shared_ptr<IScheduler> s, std::string creator, JobManager& manager);

 public:
  void setWorkingDirectory(const std::string& wdir) { working_directory = wdir; }
  void setSetupScript(const std::string& s) { setup = s; }
  void setRunScript(const std::string& r) { run = r; }
  void setTeardownScript(const std::string& t) { teardown = t; }
  void setKillOnError(bool val) { killOnError = val; }
  void setExpectedRuntime(long val) { expectedRunTime = val; }
  void setRunBeforeMainApplication(bool val) { beforeApplication = val; }
  void setHasVnVReports(bool val) { hasVnVReports = val;}

  std::string getName() const { return name; }
  std::string getWorkingDirectory() const { return working_directory; }
  std::string getSetupScript() const { return setup; }
  std::string getRunScript() const { return run; }
  std::string getTeardownScript() const { return teardown; }
  bool getHasVnVReports() const { return hasVnVReports; }
  bool runBeforeApplication() const { return beforeApplication; }

  long getExpectedRuntime() const { return expectedRunTime; }

  std::string getCreator() const { return jcreator; }

  const std::vector<std::string>& getOutputFiles() const { return outputfiles; }
  void addOutputFile(std::string filename) { outputfiles.push_back(filename); }

  JobManager& getJobManager() const { return jstore; }

  bool errorKillsWorkflow() { return killOnError; }

  long getStartTime() const { return start_time; }
  long getEndTime() const { return end_time; }

  void addEnvironmentVariable(const std::string& key, const std::string& value) { enviornment[key] = value; }
  const std::map<std::string, std::string>& getEnvironmentVariables() { return enviornment; }

  void addSuccessRequirement(std::shared_ptr<IValidator> v) { success_requirements.push_back(v); }
  const std::vector<std::shared_ptr<IValidator>>& getSuccessRequirements() { return success_requirements; }

  void addDepdendency(std::shared_ptr<IValidator> v) { dependencies.push_back(v); }
  const std::vector<std::shared_ptr<IValidator>>& getDependencies() { return dependencies; }

  std::string getVnVJobName() const { 
      return vnv_workflow_id; }

  std::string getVnVJobDir() const { return vnvJobDir.empty() ? getWorkingDirectory() : vnvJobDir; }

  std::shared_ptr<IScheduler> getScheduler() const { return scheduler; }

  JobStatus getStatus() const { return status; }

  enum class SuccessStatus { TRUE, FALSE, RUNNING, ERROR, CANCELLED };

  IValidator::Status wasSuccessful() const {
    for (auto it : success_requirements) {
      auto r = it->validate(*this);
      if (r != IValidator::Status::VALID) {
        return r;
      }
    }
    return IValidator::Status::VALID;
  }

  IValidator::Status dependenciesMet() const {
    for (auto it : dependencies) {
      auto r = it->validate(*this);
      if (r != IValidator::Status::VALID) {
        return r;
      }
    }
    return IValidator::Status::VALID;
  }

  JobStatus cancel(bool /* cancel_sub_jobs */) {
    status.state = JobStatus::State::CANCELLED;
    if (jobId.compare("__") == 0) {
      scheduler->cancel(jobId);
    }
    for (auto& it : dependencies) {
      it->cancel(*this);
    }
    for (auto& it : success_requirements) {
      it->cancel(*this);
    }
    return status;
  }

  long lastCheckedTime = 0;
  long firstCheckedTime = -1;

  long getDelay(long ms) {
    if (firstCheckedTime < 0) {
      firstCheckedTime = ms;
    }

    long max = (expectedRunTime > 60) ? 60 : expectedRunTime;
    long min = (expectedRunTime > 10) ? 10 : expectedRunTime;
    return (ms - firstCheckedTime) * (ms - firstCheckedTime) * ((min - max) / (expectedRunTime * expectedRunTime)) +
           max;
    // Delay is a parabola between first
  }

  bool delay() {
    long ms =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if ((ms - lastCheckedTime) > getDelay(ms)) {
      lastCheckedTime = ms;
      return true;
    }
    return false;
  }

  JobStatus progress() {
    if (status.state == JobStatus::State::WAITING_FOR_LAUNCH) {
      VnV_Info(VNVPACKAGENAME, "Job %s: Validating Dependencies", getName().c_str());
      start_time = std::time(0);
      status.state = JobStatus::State::CHECKING_DEPENDENCIES;
    }

    if (status.state == JobStatus::State::CHECKING_DEPENDENCIES) {
      auto a = dependenciesMet();
      if (a == IValidator::Status::VALID) {
        VnV_Info(VNVPACKAGENAME, "Job %s Dependencies Validated - Starting Job", getName().c_str());
        status.state = JobStatus::State::LAUNCHING_JOB;
      } else if (a == IValidator::Status::INVALID) {
        VnV_Info(VNVPACKAGENAME, "Job %s: Dependency Validation Failed -- Job will not run.  ", getName().c_str());
        status.state = JobStatus::State::ERROR;
      }
    }

    if (status.state == JobStatus::State::LAUNCHING_JOB) {
      status.state = JobStatus::State::RUNNING;
      status.jobId = scheduler->run(run, working_directory, enviornment, setup, teardown, -1);
    }

    if (status.state == JobStatus::State::RUNNING) {
      // Throw in some sort of delay.
      if (!delay()) {
        status.result = scheduler->status(status.jobId);
        if (status.result.status != ProcessResult::Status::RUNNING) {
          status.state = JobStatus::State::CHECKING_SUCCESS_REQUIREMENTS;
          VnV_Info(VNVPACKAGENAME, "Job %s: Finished Executing (exit code: %ld) - Checking Success Requirements  ",
                   getName().c_str(), status.result.exitCode);
        }
      }
    }

    if (status.state == JobStatus::State::CHECKING_SUCCESS_REQUIREMENTS) {
      auto a = wasSuccessful();
      if (a == IValidator::Status::VALID) {
        VnV_Info(VNVPACKAGENAME, "Job %s: Success Requirements Satified - Job Ran Successfully.", getName().c_str());
        status.state = JobStatus::State::JOB_FINISHED;
      } else if (a == IValidator::Status::INVALID) {
        status.state = JobStatus::State::ERROR;
      }
    }

    if (status.state == JobStatus::State::JOB_FINISHED) {
      end_time = std::time(0);
      status.state = JobStatus::State::COMPLETE;
      VnV_Info(VNVPACKAGENAME, "Job %s: Job complated in %ld seconds ", getName().c_str(), end_time - start_time);
    }

    return status;
  }
};

class JobManager {
  std::map<std::string, std::shared_ptr<Job>> jobs;
  std::string package, name;
  std::stack<std::string> creators;

 public:
  static std::string findName(std::string pre, std::map<std::string, nlohmann::json>& nodes) {
    int i = 0;
    std::string name;
    do {
      name = pre + "_" + std::to_string(i++);
    } while (nodes.find(name) != nodes.end());
    return name;
  }

  void pushCreator(std::string package, std::string name) {
      creators.push(package + ":" + name);
  }
  void popCreator() {
      creators.pop();
  }

  template <typename T>
  static std::string addnode(std::string type, const T value, std::map<std::string, nlohmann::json>& nodes) {
    std::string name = findName(type, nodes);
    json j = json::object();
    j["id"] = name;
    j["type"] = type;
    j["value"] = value;
    nodes[name] = j;
    return name;
  }

  // Add a link between a node and a job.
  static void add_job_link(std::string jobToLinkTo, std::string nodeToLinkTo, std::string jobCreatingLink,
                           bool validation, std::map<std::string, nlohmann::json>& nodes) {
    json j = json::object();
    j["node"] = nodeToLinkTo;
    j["jobName"] = jobToLinkTo;
    j["creator"] = jobCreatingLink;
    j["validation"] = validation;
    nodes["__job_links__"].push_back(j);
  }

  static std::string add_file_node(std::string filename, std::map<std::string, nlohmann::json>& nodes) {
    for (auto& it : nodes) {
      if (it.second.is_object() && it.second.value("type", "").compare("File") == 0) {
        if (it.second.value("value", "") == filename) {
          return it.first;
        }
      }
    }
    return addnode("File", filename, nodes);
  }

  JobManager(std::string package, std::string name) {
    this->package = package;
    this->name = name;
    creators.push(package + ":" + name);
  }

  std::shared_ptr<Job> create(std::string name, std::shared_ptr<IScheduler> scheduler) {
    auto it = jobs.find(name);
    if (it == jobs.end()) {
      std::shared_ptr<Job> j;
      j.reset(new Job(name, scheduler, creators.top(), *this));
      jobs[name] = j;

      return j;
    }
    throw INJECTION_EXCEPTION_("Job by that name already exists.");
  }

  void subjob(std::string package, std::string name, const json& config);

  std::shared_ptr<Job> get(std::string name) {
    auto it = jobs.find(name);
    if (it != jobs.end()) {
      return it->second;
    }
    throw INJECTION_EXCEPTION("No job by the name %s exists", name.c_str());
  }

  void run(ICommunicator_ptr ptr, bool before);

  static void add_edge(std::string source, std::string target, json& edges) {
    json j = json::object();
    j["source"] = source;
    j["target"] = target;
    edges.push_back(j);
  }

  json runInfo;

  std::tuple<std::string, std::string, bool> add_node(std::shared_ptr<Job> job, std::map<std::string, json>& nodes,
                                                      json& edges);

  json getWorkflowFile() {
    json edges = json::array();
    std::map<std::string, nlohmann::json> nodes;

    nodes["__job_links__"] = json::array();

    // Add the main application node. That is the node the ran the simulation.
    // The main application runs after the simulation is complete.
    std::string mainApp = addnode("MainApplication", "Main", nodes);

    std::map<std::string, std::tuple<std::string, std::string, bool>> nodeMap;
    for (auto job : jobs) {
      nodeMap[job.second->getName()] = add_node(job.second, nodes, edges);
    }

    // Add the job links after so we know they are there.
    // Also add links to the main application for any before nodes with no children
    // and any after nodes with no parents.
    std::set<std::string> hasParents, hasChildren;

    for (auto it : nodes["__job_links__"]) {
      std::string jobName = it["jobName"].get<std::string>();

      auto fi = nodeMap.find(jobName);

      if (fi != nodeMap.end()) {
        std::string jobNode = it["validation"].get<bool>() ? std::get<1>(fi->second) : std::get<0>(fi->second);
        std::string nodeNode = it["node"].get<std::string>();

        // JobNode is a parent of nodeNode.
        // ---> The job has a child and should not be linked to the Main Application as a before node.
        add_edge(jobNode, nodeNode, edges);
        hasChildren.insert(fi->first);

        // Also, the job responsible for creating nodeNode has a job as parent, so it
        // also shouldn't be linked to the main application node.
        hasParents.insert(it["creator"].get<std::string>());
      }
    }
    nodes.erase("__job_links__");

    for (auto it : nodeMap) {
      std::string& jobNode = std::get<0>(it.second);
      bool& before = std::get<2>(it.second);

      if (before && hasChildren.find(it.first) == hasChildren.end()) {
        add_edge(jobNode, mainApp, edges);
      } else if (!before && hasParents.find(it.first) == hasParents.end()) {
        add_edge(mainApp, jobNode, edges);
      }
    }

    json narray = json::array();
    for (auto it : nodes) {
      narray.push_back(it.second);
    }

    json r = json::object();
    r["nodes"] = narray;
    r["links"] = edges;
    return r;
  }
};

// Might expand on this later.
// Chuck it in a class just in case.
class Script {
  std::string script;

 public:
  Script(std::string s) : script(s) {}
  std::string generate() { return script; }
};

class IJobCreator {
 public:
  virtual void createJob(JobManager& manager) = 0;
  virtual ~IJobCreator() {}
};

// Little plugin to allow for script generation (e.g. for moab scripts. )
class IScriptGenerator {
 public:
  virtual std::shared_ptr<Script> generateScript() = 0;
  virtual ~IScriptGenerator() {}

  // Built in helper function to support generating scripts using the inja C++ template engine.
  // For a tutorial on inja, please see ( https://pantor.github.io/inja/ )
  static std::string render(std::string templ, const nlohmann::json& data);
};

typedef IJobCreator* (*job_ptr)(const nlohmann::json& config);
void registerJobCreator(std::string packageName, std::string name, const std::string schema, job_ptr r);

typedef IValidator* (*validator_ptr)(const nlohmann::json& config);
void registerValidator(std::string packageName, std::string name, const std::string schema, validator_ptr r);

typedef IScriptGenerator* (*script_gen_ptr)(const nlohmann::json& config);
void registerScriptGenerator(std::string packageName, std::string name, const std::string schema, script_gen_ptr r);

typedef IScheduler* (*scheduler_ptr)(const nlohmann::json& config);
void registerScheduler(std::string packageName, std::string name, const std::string schema, scheduler_ptr r);

class InjaScriptGenerator : public IScriptGenerator {
  std::string script;

 public:
  InjaScriptGenerator(std::string template_, const nlohmann::json& config) { script = render(template_, config); }
  virtual std::shared_ptr<Script> generateScript() override { return std::make_shared<Script>(script); };
  virtual ~InjaScriptGenerator() {}
};

}  // namespace VnV

#define INJECTION_JOBCREATOR(PNAME, name, SCHEMA)                                                     \
  namespace VnV {                                                                                     \
  namespace PNAME {                                                                                   \
  namespace JobCreators {                                                                             \
  class name : public VnV::IJobCreator {                                                              \
    nlohmann::json config;                                                                            \
                                                                                                      \
   public:                                                                                            \
    name(const nlohmann::json& c) : config(c) {}                                                      \
    void createJob(JobManager& jobmanager) override;                                                  \
  };                                                                                                  \
                                                                                                      \
  VnV::IJobCreator* declare_##name(const nlohmann::json& config) { return new name(config); }         \
  void register_##name() { VnV::registerJobCreator(VNV_STR(PNAME), #name, SCHEMA, &declare_##name); } \
  }                                                                                                   \
  }                                                                                                   \
  }                                                                                                   \
  void VnV::PNAME::JobCreators::name::createJob(JobManager& manager)



#define INJECTION_CREATE_JOB(VAR,NAME,SCHEDULER)\
    auto VAR = manager.create(NAME,SCHEDULER);



#define DECLAREJOBCREATOR(PNAME, name) \
  namespace VnV {                      \
  namespace PNAME {                    \
  namespace JobCreators {              \
  void register_##name();              \
  }                                    \
  }                                    \
  }

#define REGISTERJOBCREATOR(PNAME, name) VnV::PNAME::JobCreators::register_##name();

#define INJECTION_VALIDATOR(PNAME, name, schema)                                                \
  namespace VnV {                                                                               \
  namespace PNAME {                                                                             \
  namespace Validators {                                                                        \
  VnV::IValidator* declare_##name(const nlohmann::json& config);                                \
  void register_##name() { registerValidator(VNV_STR(PNAME), #name, schema, &declare_##name); } \
  }                                                                                             \
  }                                                                                             \
  }                                                                                             \
  VnV::IValidator* VnV::PNAME::Validators::declare_##name(const nlohmann::json& config)

#define DECLAREVALIDATOR(PNAME, name) \
  namespace VnV {                     \
  namespace PNAME {                   \
  namespace Validators {              \
  void register_##name();             \
  }                                   \
  }                                   \
  }

#define REGISTERVALIDATOR(PNAME, name) VnV::PNAME::Validators::register_##name();

#define INJECTION_SCRIPTGENERATOR(PNAME, name, SCHEMA)                                                     \
  namespace VnV {                                                                                          \
  namespace PNAME {                                                                                        \
  namespace Scripts {                                                                                      \
                                                                                                           \
  VnV::IScriptGenerator* declare_##name(const nlohmann::json& config);                                     \
  void register_##name() { VnV::registerScriptGenerator(VNV_STR(PNAME), #name, SCHEMA, &declare_##name); } \
  }                                                                                                        \
  }                                                                                                        \
  }                                                                                                        \
  VnV::IScriptGenerator* VnV::PNAME::Scripts::declare_##name(const nlohmann::json& config)

#define DECLARESCRIPTGENERATOR(PNAME, name) \
  namespace VnV {                           \
  namespace PNAME {                         \
  namespace Scripts {                       \
  void register_##name();                   \
  }                                         \
  }                                         \
  }

#define REGISTERSCRIPTGENERATOR(PNAME, name) VnV::PNAME::Scripts::register_##name();

// Little shortcut for inja templates.
#define INJECTION_INJA_TEMPLATE(PACKAGE, NAME, SCHEMA, TEMPL) \
  INJECTION_SCRIPTGENERATOR(PACKAGE, NAME, SCHEMA) { return new VnV::InjaScriptGenerator(TEMPL, config); }

#define INJECTION_SCHEDULER(PNAME, name, schema)                                                \
  namespace VnV {                                                                               \
  namespace PNAME {                                                                             \
  namespace Schedulers {                                                                        \
  VnV::IScheduler* declare_##name(const nlohmann::json& config);                                \
  void register_##name() { registerScheduler(VNV_STR(PNAME), #name, schema, &declare_##name); } \
  }                                                                                             \
  }                                                                                             \
  }                                                                                             \
  VnV::IScheduler* VnV::PNAME::Schedulers::declare_##name(const nlohmann::json& config)

#define DECLARESCHEDULER(PNAME, name) \
  namespace VnV {                     \
  namespace PNAME {                   \
  namespace Schedulers {              \
  void register_##name();             \
  }                                   \
  }                                   \
  }

#define REGISTERSCHEDULER(PNAME, name) VnV::PNAME::Schedulers::register_##name();

#endif
