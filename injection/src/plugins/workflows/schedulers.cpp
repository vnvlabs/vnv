#include <chrono>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "interfaces/IWorkflow.h"

namespace {

class BashScheduler : public VnV::IScheduler {
 public:
  std::map<std::string, std::shared_ptr<VnV::DistUtils::VnVProcess>> processes;

  virtual std::string run(std::string script, std::string workingDirectory,
                          const std::map<std::string, std::string>& environ, std::string setup, std::string teardown,
                          int timeout) override {
    std::string s = VnV::StringUtils::random(5);

    //First, we write there script to file. 
    
    std::ostringstream os;
    os << "#!/bin/env bash \n\n"
       << "cd " << workingDirectory << " \n";
    
    for (auto it : environ) {
      os << "export " << it.first << "=" << it.second << "\n";
    }
    os << "\n\n" << setup << "\n\n" << script << "\n\n" << teardown << "\n\n";

    // Execute the script. 
    auto p = VnV::DistUtils::exec(os.str());

    //Add it to our database of processes 
    std::string ra = VnV::StringUtils::random(5);
    while (processes.find(ra) != processes.end()) {
      ra = VnV::StringUtils::random(5);
    }
    processes[ra] = p;

    // Wait until timeout expires or until it finishes running 
    auto t = std::chrono::system_clock::now();
    while (true) {
      auto n = std::chrono::system_clock::now() - t;
      if (std::chrono::duration_cast<std::chrono::seconds>(n).count() > timeout || !p->running()) {
        return ra;
      }
    }
  }

  virtual std::string stage(std::string source, std::string dest, VnV::IScheduler::CopyType type,
                            std::string working_directory) override {
    std::string comm = "";
    switch (type) {
    case VnV::IScheduler::CopyType::TEXT:
      comm = "cat << __STAGE__ > " + dest + "\n" + source + "\n__STAGE__\n";
      break;
    case VnV::IScheduler::CopyType::COPY:
      comm = "cp " + source + " " + dest;
      break;
    case VnV::IScheduler::CopyType::MOVE:
      comm = "mv " + source + " " + dest;
      break;
    case VnV::IScheduler::CopyType::HARD_LINK:
      comm = "ln " + source + " " + dest;
      break;
    case VnV::IScheduler::CopyType::SOFT_LINK:
      comm = "ln -s" + source + " " + dest;
      break;
    }

    return run(comm, working_directory, {}, "", "", -1);
  }

  virtual void cancel(std::string jobId) override {
    auto rr = processes.find(jobId);
    if (rr != processes.end()) {
      rr->second->cancel();
    }
  }

  virtual VnV::ProcessResult status(std::string jobId, long timeout = -1) override {
    
    auto rr = processes.find(jobId);
    if (rr != processes.end()) {
      
      if (timeout > 0) {
            auto t = std::chrono::system_clock::now();
            while (!rr->second->running() && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - t ).count() < timeout) {}
      }
      
      if (rr->second->running()) {    
          return VnV::ProcessResult(VnV::ProcessResult::Status::RUNNING, -1,"","");
      
      } else {
      
          return VnV::ProcessResult(
              VnV::ProcessResult::Status::SUCCESS, 
              rr->second->getExitStatus(),
              rr->second->getStdout(), 
              rr->second->getStdError());
      
      }

    }
    throw INJECTION_EXCEPTION("Unknown Job %s", jobId.c_str());
  }

  virtual bool exists(std::string filename, std::string working_directory) override {
    std::string fname = VnV::DistUtils::join({working_directory, filename}, 0777, false);
    return VnV::DistUtils::fileExists(fname);
  }

  virtual ~BashScheduler() {}
};

class SlurmScheduler : public BashScheduler {
 public:
  std::string tempDir = "/tmp";
  std::map<std::string, std::shared_ptr<VnV::DistUtils::VnVProcess>> sprocesses;

  virtual std::string run(std::string script, std::string workingDirectory,
                          const std::map<std::string, std::string>& environ, std::string setup, std::string teardown,
                          int timeout) override {
    std::string s = VnV::StringUtils::random(5);
    while (processes.find(s) != processes.end() || sprocesses.find(s) != sprocesses.end()) {
      s = VnV::StringUtils::random(5);
    }

    std::ostringstream os;

    os << "#!/bin/env bash \n\n cd " << workingDirectory << " \n";
    for (auto it : environ) {
      os << "export " << it.first << "=" << it.second << "\n";
    }
    os << "\n\n" << setup << "\n\n";

    os << "cat <<- EOF > " << tempDir << "/" << s << ".slurm \n";
    os << script << "\n";
    os << "EOF";
    os << "qsub " << s << ".slurm > " << tempDir << "/" << s << ".sjid \n";
    os << teardown;
    // Stage the file and wait for it to finish.

    std::string st = stage(os.str(), tempDir + "/" + s + ".sh", VnV::IScheduler::CopyType::TEXT, workingDirectory);
    processes.find(st)->second->wait();

    std::ostringstream oss;
    oss << "cd " << workingDirectory << " && chmod u+x " << tempDir << "/" << s << ".sh"
        << " && " << tempDir << "/" << s << ".sh ";

    auto r = VnV::DistUtils::exec(oss.str());

    sprocesses[s] = r;
    return s;
  }

  virtual std::string getSlurmJobId(std::string jobId) {
    std::ifstream iff(tempDir + "/" + jobId + ".sjid");
    if (iff.good()) {
      std::stringstream bugg;
      bugg << iff.rdbuf();
      return bugg.str();
    }

    throw INJECTION_EXCEPTION_("Could not open jobId");
  }

  virtual VnV::ProcessResult processSlurmResult(std::string jobId) {
    auto r = BashScheduler::run("qstat " + getSlurmJobId(jobId), ".", {}, "", "", -1);
    auto a = processes[r];
    a->wait();
    std::string stdo = a->getStdout();
    std::string stde = a->getStdError();
    int exitStat = a->getExitStatus();
    return VnV::ProcessResult(VnV::ProcessResult::Status::SUCCESS,exitStat,stdo,stde);;  // TODO
  }

  virtual void cancel(std::string jobId) override {
    auto rr = sprocesses.find(jobId);
    if (rr != sprocesses.end()) {
      rr->second->cancel();
      auto r = BashScheduler::run("qdel " + getSlurmJobId(jobId), ".", {}, "", "", -1);
    } else {
      BashScheduler::cancel(jobId);
    }
  }

  virtual VnV::ProcessResult status(std::string jobId, long timeout = -1 ) override {
    auto rr = sprocesses.find(jobId);
    if (rr != sprocesses.end()) {
      
       if (timeout > 0) {
            auto t = std::chrono::system_clock::now();
            while (!rr->second->running() && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - t ).count() < timeout) {}
       }
      
       if (rr->second->running()) {    
          return VnV::ProcessResult(VnV::ProcessResult::Status::RUNNING, -1,"","");
      } else {
          return processSlurmResult(jobId); 
      }    
    }
    return BashScheduler::status(jobId);
  }

  virtual bool exists(std::string filename, std::string working_directory) override {
    std::string fname = VnV::DistUtils::join({working_directory, filename}, 0777, false);
    return VnV::DistUtils::fileExists(fname);
  }

  virtual ~SlurmScheduler() {}
};

}  // namespace

INJECTION_SCHEDULER(VNVPACKAGENAME, Bash, "{}") { return new BashScheduler(); }
INJECTION_SCHEDULER(VNVPACKAGENAME, Slurm, "{}") { return new SlurmScheduler(); }
