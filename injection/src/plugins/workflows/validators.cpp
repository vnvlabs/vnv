#include <chrono>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/stores/WorkflowStore.h"
#include "interfaces/IWorkflow.h"

/**************************BOOLEAN VALIDATOR ****************/

namespace {}  // namespace

class BooleanValidator : public VnV::IValidator {
  bool val = true;

 public:
  BooleanValidator(bool value) {}
  virtual Status validate(const VnV::Job& job) override { return val ? Status::VALID : Status::INVALID; };

  std::string addNodesAndEdges(const VnV::Job& job, std::map<std::string, nlohmann::json>& nodes,
                               nlohmann::json& edges) override {
    return VnV::JobManager::addnode("Boolean", val, nodes);
  }

  virtual void cancel(const VnV::Job& job) override{};
  virtual ~BooleanValidator() {}
};

INJECTION_VALIDATOR(VNVPACKAGENAME, Boolean,
                    R"({"type" : "object" , "properties" : {"value" : {"type" : "boolean"} } })") {
  return new BooleanValidator(config.value("value", true));
}

/**************************AND OR VALIDATOR ****************/

class AndOrValidator : public VnV::IValidator {
  std::shared_ptr<VnV::IValidator> one, two;
  bool a = false;
  std::vector<std::pair<bool, std::shared_ptr<VnV::IValidator>>> subs;

 public:
  AndOrValidator(bool andOp, std::shared_ptr<VnV::IValidator> first, std::shared_ptr<VnV::IValidator> second)
      : one(first), two(second) {
    a = andOp;
  }

  virtual Status validate(const VnV::Job& job) override {
    Status valid = one->validate(job);
    Status valid2 = two->validate(job);

    if (a) {
      if (valid == Status::WAIT || valid2 == Status::WAIT) {
        return Status::WAIT;
      }
      if (valid == Status::INVALID || valid2 == Status::INVALID) {
        return Status::INVALID;
      }
      return Status::VALID;
    }

    if (valid == Status::VALID || valid2 == Status::VALID) {
      return Status::VALID;
    }
    if (valid == Status::INVALID && valid2 == Status::INVALID) {
      return Status::INVALID;
    }
    return Status::WAIT;
  }

  std::string addNodesAndEdges(const VnV::Job& job, std::map<std::string, nlohmann::json>& nodes,
                               nlohmann::json& edges) override {
    std::string name = VnV::JobManager::addnode("AndOr", a ? "And" : "Or", nodes);
    std::string e1 = one->addNodesAndEdges(job, nodes, edges);
    std::string e2 = two->addNodesAndEdges(job, nodes, edges);
    VnV::JobManager::add_edge(e1, name, edges);
    VnV::JobManager::add_edge(e2, name, edges);
    return name;
  }

  virtual void cancel(const VnV::Job& job) override{};
  virtual ~AndOrValidator() {}

  static VnV::IValidator* create(const json& config) {
    std::shared_ptr<VnV::IValidator> s1, s2;

    for (auto it : config["first"].items()) {
      std::vector<std::string> p;
      VnV::StringUtils::StringSplit(it.key(), ":", p);
      s1 = VnV::WorkflowStore::instance().getValidator(p[0], p[1], it.value());
      break;
    }

    for (auto it : config["second"].items()) {
      std::vector<std::string> p;
      VnV::StringUtils::StringSplit(it.key(), ":", p);
      s2 = VnV::WorkflowStore::instance().getValidator(p[0], p[1], it.value());
      break;
    }

    return new AndOrValidator(config.value("and", true), s1, s2);
  }
};

INJECTION_VALIDATOR(VNVPACKAGENAME, AndOr, R"({
    "type" : "object",
    "properties" : {
        "and" : {"type" : "boolean", "default" : true },
        "first" : { "type" : "object" },
        "second" : { "type" : "object" }
    },
    "required" : ["first","second" ]
})") {
  return AndOrValidator::create(config);
}

/******************* FILE EXISTS ************************/

class FileValidator : public VnV::IValidator {
  std::string f, r, req;
  bool w;

 public:
  FileValidator(std::string filename, std::string req_ = "exists", std::string reader = "", bool wait = true)
      : f(filename), req(req_), r(reader), w(wait) {}

  virtual Status validate(const VnV::Job& job) override {
    if (req == "exists") {
      return VnV::DistUtils::fileExists(f) ? Status::VALID : (w ? Status::WAIT : Status::INVALID);
    } else if (req == "notExists") {
      return !VnV::DistUtils::fileExists(f) ? Status::VALID : (w ? Status::WAIT : Status::INVALID);
    } else {
      throw INJECTION_EXCEPTION("Validation Error for req %s", req.c_str());
    }
  }

  std::string addNodesAndEdges(const VnV::Job& job, std::map<std::string, nlohmann::json>& nodes,
                               nlohmann::json& edges)  override {
    std::string fileExistsnode = VnV::JobManager::addnode("FileExists", req, nodes);
    std::string fileNode = VnV::JobManager::add_file_node(f, nodes);
    VnV::JobManager::add_edge(fileNode, fileExistsnode, edges);
    return fileExistsnode;
  }

  virtual void cancel(const VnV::Job& job) override{};
  virtual ~FileValidator() {}
};

INJECTION_VALIDATOR(VNVPACKAGENAME, File, R"({
    "type" : "object",
    "properties" : {
        "filename" : {"type":"string"},
        "wait" : {"type" : "boolean" },
        "requirement" : {"type":"string","enum":["exists","notExists","changed","notChanged"]},
        "reader" : {"type" : "string" }
     },
    "required" : ["filename"]
})") {
  return new FileValidator(config["filename"].get<std::string>(), config.value("requirement", "exists"),
                           config.value("reader", "auto"), config.value("wait", true));
}

class JobFinishedValidator : public VnV::IValidator {
  int val = true;
  std::string comp, jobname;

 public:
  JobFinishedValidator(std::string jobName, std::string c = "", int value = 0)
      : jobname(jobName), val(value), comp(c) {}

  virtual Status validate(const VnV::Job& job) override {
    int exitStatus = -1;
    VnV::JobStatus::State s;

    if (jobname.empty()) {
      // In this case, we are checking the return status of another job.
      s = job.getStatus().state;
      exitStatus = job.getStatus().result.exitCode;

      // If this is a self job -- I.e., the validator is applied to the job
      // that is being validated, then we HAVE to be in the state of
      // Checking success requirements. In other words, the job has to
      // run before we can validate the success.
      if (s != VnV::JobStatus::State::CHECKING_SUCCESS_REQUIREMENTS) {
        return Status::INVALID;
      }

    } else {
      // In this case, we are checking the return status of some other job.
      auto j = job.getJobManager().get(jobname);
      s = j->getStatus().state;

      if (s == VnV::JobStatus::State::ERROR) {
        return Status::INVALID;
      }

      if (s != VnV::JobStatus::State::COMPLETE) {
        return Status::WAIT;
      }
      exitStatus = j->getStatus().result.exitCode;
    }

    bool r = false;
    if (comp.empty()) {
      r = true;
    } else if (comp.compare("==") == 0) {
      r = exitStatus == val;
    } else if (comp.compare("<") == 0) {
      r = exitStatus < val;
    } else if (comp.compare(">") == 0) {
      r = exitStatus > val;
    } else if (comp.compare("<=") == 0) {
      r = exitStatus <= val;
    } else if (comp.compare(">=") == 0) {
      r = exitStatus >= val;
    } else if (comp.compare("!=") == 0) {
      r = exitStatus != val;
    } else {
      r = false;
    }
    return r ? Status::VALID : Status::INVALID;
  }

  std::string addNodesAndEdges(const VnV::Job& job, std::map<std::string, nlohmann::json>& nodes,
                               nlohmann::json& edges) override {
    std::string value = comp.empty() ? "Job Completed" : "Exit Status " + comp + " " + std::to_string(val);
    std::string name = VnV::JobManager::addnode("JobDependency", value, nodes);

    auto actualj = (jobname.empty()) ? job : *(job.getJobManager().get(jobname));
    VnV::JobManager::add_job_link(actualj.getName(), name, job.getName(), false, nodes);
    return name;
  }

  virtual void cancel(const VnV::Job& job) override{

  };

  virtual ~JobFinishedValidator() {}
};

INJECTION_VALIDATOR(VNVPACKAGENAME, ExitCode, R"({
    "type" : "object",
    "properties" : {
        "job" : {"type":"string"},
        "compare" : {"type":"string","enum":["<","<=","==",">",">=","!=",""]},
        "value" : {"type" : "integer", "default":0 }
     }
})") {
  return new JobFinishedValidator(config.value("job", ""), config.value("compare", "=="), config.value("value", 0));
}

class FileStagingValidator : public VnV::IValidator {
  int val = true;
  std::string src, dest, wdir, jobId;
  VnV::IScheduler::CopyType copyType;

 public:
  FileStagingValidator(std::string source, std::string destination, VnV::IScheduler::CopyType type,
                       std::string workDir = "")
      : wdir(workDir), src(source), dest(destination), copyType(type) {}

  virtual Status validate(const VnV::Job& job) override {
    std::string w = wdir.empty() ? job.getWorkingDirectory() : wdir;

    if (jobId.empty()) {
      jobId = job.getScheduler()->stage(src, dest, copyType, w);
      return Status::WAIT;
    }
    auto a = job.getScheduler()->status(jobId);
    if (a.status == VnV::ProcessResult::Status::RUNNING) {
      return Status::WAIT;
    }

    if (a.status == VnV::ProcessResult::Status::SUCCESS) {
      // If successful, make sure the file exists and return result according;y
      std::string p = VnV::DistUtils::join({w, dest}, 0777, false);
      return VnV::DistUtils::fileExists(p) ? Status::VALID : Status::INVALID;

    } else {
      return Status::INVALID;
    }
  }

  std::string addNodesAndEdges(const VnV::Job& job, std::map<std::string, nlohmann::json>& nodes,
                               nlohmann::json& edges) override {
    std::string w = wdir.empty() ? job.getWorkingDirectory() : wdir;

    std::string abs_dest = VnV::DistUtils::join({w, dest}, 0777, false);

    std::string abs_src =
        (copyType == VnV::IScheduler::CopyType::TEXT) ? src : VnV::DistUtils::join({w, src}, 0777, false);

    std::string name = VnV::JobManager::addnode("FileStaging", "", nodes);
    std::string srcfilenode = VnV::JobManager::add_file_node(abs_src, nodes);
    std::string destfilenode = VnV::JobManager::add_file_node(abs_dest, nodes);

    VnV::JobManager::add_edge(srcfilenode, name, edges);
    VnV::JobManager::add_edge(name, destfilenode, edges);
    return destfilenode;
  }

  virtual void cancel(const VnV::Job& job) override {
    if (!jobId.empty()) {
      job.getScheduler()->cancel(jobId);
    }
  }

  virtual ~FileStagingValidator() {}
};

INJECTION_VALIDATOR(VNVPACKAGENAME, FileStaging, R"({
    "type" : "object",
    "properties" : {
        "src" : {"type":"string"},
        "dest" : {"type":"string"},
        "wdir" : {"type":"string"},
        "type" : {"type" : "string", "enum":["copy","move","hard link","soft link","text","asis"] }
     },
    "required" : ["src","dest"]
})") {
  VnV::IScheduler::CopyType type = VnV::IScheduler::CopyType::COPY;
  std::string c = config.value("type", "text");

  if (c.compare("copy") == 0) {
    type = VnV::IScheduler::CopyType::COPY;
  } else if (c.compare("move") == 0) {
    type = VnV::IScheduler::CopyType::MOVE;
  } else if (c.compare("hard link") == 0) {
    type = VnV::IScheduler::CopyType::HARD_LINK;
  } else if (c.compare("soft line") == 0) {
    type = VnV::IScheduler::CopyType::SOFT_LINK;
  } else if (c.compare("text") == 0) {
    type = VnV::IScheduler::CopyType::TEXT;
  } else if (c.compare("asis") == 0) {
    type = VnV::IScheduler::CopyType::ASIS;
  }

  return new FileStagingValidator(config["src"].get<std::string>(), config["dest"].get<std::string>(), type,
                                  config.value("wdir", ""));
}
