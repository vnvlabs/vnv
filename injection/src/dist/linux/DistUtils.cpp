
/**
  @file DistUtils.cpp
**/

#include "base/DistUtils.h"

#include <dirent.h>
#include <errno.h>
#include <link.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "base/Utilities.h"
#include "base/exceptions.h"

using nlohmann::json;
namespace VnV {
namespace DistUtils {

libInfo getLibInfo(std::string filepath, unsigned long add) {
  libInfo info;

  struct stat result;
  stat(filepath.c_str(), &result);
  json libJson;
  info.name = getAbsolutePath(filepath);
  info.size = result.st_size;
  info.timestamp = result.st_mtim.tv_sec;
  return info;
}

char* getCurrentDirectory() { return get_current_dir_name(); }

void initialize_lock(LockFile* lockfile) {}

void lock_file(LockFile* lockfile) { flock(lockfile->fd, LOCK_EX); }

void unlock_file(LockFile* lockfile) { flock(lockfile->fd, LOCK_UN); }

void close_file(LockFile* lockfile) {
  unlock_file(lockfile);
  close(lockfile->fd);
}

std::string getAbsolutePath(std::string filename) {
  char* x = realpath(filename.c_str(), nullptr);
  if (x != nullptr) {
    std::string path(x);
    free(x);
    return path;
  }
  return filename;
}

bool fileEquals(std::string f1, std::string f2) { return getAbsolutePath(f1).compare(getAbsolutePath(f2)) == 0; }

bool fileInDirectory(std::string file, std::string directory) {
  auto ap = getAbsolutePath(file);
  auto ad = getAbsolutePath(directory);
  return ap.rfind(ad, 0) == 0;
}

bool makedir(std::string filename, mode_t mode) { return mkdir(filename.c_str(), mode) == 0; }

bool mv(std::string oldFileName, std::string newFileName) {
  return 0 == std::rename(oldFileName.c_str(), newFileName.c_str());
}

bool cp(std::string oldFileName, std::string newFilename) {
  // Gotta be a better way than this right?
  std::ifstream src(oldFileName, std::ios::binary);
  std::ofstream dst(newFilename, std::ios::binary);
  dst << src.rdbuf();
  return true;
}
bool ln(std::string oldFileName, std::string newFilename, bool hard) {
  if (hard) {
    link(oldFileName.c_str(), newFilename.c_str());
  } else {
    symlink(oldFileName.c_str(), newFilename.c_str());
  }
  return true;
}

std::string getTempFolder() {
  char const* folder = getenv("TMPDIR");
  if (!folder) folder = getenv("TMP");
  if (!folder) folder = getenv("TMP");
  if (!folder) folder = getenv("TEMP");
  if (!folder) folder = getenv("TEMPDIR");
  if (!folder) folder = "/tmp";
  return folder;
}

class ActualProcess {
 public:
  pid_t pid;

  ActualProcess(std::string command) {
    pid_t child_pid;

    if ((child_pid = fork()) == -1) {
      perror("fork");
      exit(1);
    }

    /* child process */
    if (child_pid == 0) {
      setpgid(child_pid, child_pid);  // Needed so negative PIDs can kill children of /bin/sh

      // Write the command to file as a temporary file.
      std::string s = getTempFile();
      std::ofstream ofs(s);
      ofs << "#!/bin/env bash \n\n" << command << "\n\n";
      ofs.close();
      permissions(s, true, true, true);  // Not sure if needed.
      execl(s.c_str(), s.c_str(), (char*)NULL);
      _exit(0);

    } else {
    }

    pid = child_pid;
  }

  int wait() {
    if (pid > -1) {
      int stat;
      while (waitpid(pid, &stat, 0) == -1) {
        if (errno != EINTR) {
          stat = -1;
          break;
        }
      }
      return stat;
    }
    return 0;
  }

  void cancel() {
    kill(pid, 9);
    pid = -1;
  }

  ~ActualProcess() { wait(); }
};

std::string getTempFile(std::string code, std::string ext) {
  return join({getTempFolder(), "vnv_" + code + ext}, 077, false);
}

class LinuxProcess : public VnVProcess {
  int exitStatus = 0;
  std::string stdo = "";
  std::string stde = "";
  std::string cfile = StringUtils::random(10);
  std::unique_ptr<ActualProcess> process;

 public:
  LinuxProcess(std::string cmd) {
    // Put it all in a script -- All stdout will be piped through
    // to a file. The exit status is piped to another file.

    // Get a unique file name
    while (fileExists(getTempFile(cfile))) {
      cfile = StringUtils::random(10);
    }

    // Write the users command into a new script file.
    std::string cfn = getTempFile(cfile);
    std::ofstream cf(cfn);
    cf << cmd;
    cf.close();

    // Make it executable
    permissions(cfn, true, true, true);

    // Execute it, mapping stdout, stderr and the exit status to cfile.stderr, cfile.stdout and cfile.stdexit
    std::ostringstream oss;
    oss << cfn << " 1>" << getTempFile(cfile, ".stdout") << " 2>" << getTempFile(cfile, ".stderr") << "; echo $? > "
        << getTempFile(cfile, ".exit");
    process.reset(new ActualProcess(oss.str()));
  }

  int getExitStatus() override {
    wait();
    return exitStatus;
  }

  std::string getStdout() override {
    wait();
    return stdo;
  }
  std::string getStdError() override {
    wait();
    return stde;
  }

  virtual void wait() override {
    if (process != nullptr) {
      process->wait();  // Calls destructor which waits for process to end.
      stdo = rfile(getTempFile(cfile, ".stdout"));
      stde = rfile(getTempFile(cfile, ".stderr"));
      exitStatus = std::atoi(rfile(getTempFile(cfile, ".exit")).c_str());
    }
  }

  std::string rfile(std::string filename) {
    std::ifstream inFile;
    inFile.open(filename.c_str());  // open the input file
    std::stringstream strStream1;
    strStream1 << inFile.rdbuf();  // read the file
    return strStream1.str();       // str holds the content of the file
  }

  virtual void cancel() override {
    if (process != nullptr) {
      process->cancel();
    }
  }

  // Still running if we cant find complete.
  virtual bool running() override {
    std::ifstream df(getTempFile(cfile, ".exit"));
    if (!df.good()) {
      return true;
    }
    return false;
  };

  virtual ~LinuxProcess() {}
};

std::shared_ptr<VnVProcess> exec(std::string cmd) { return std::make_shared<LinuxProcess>(cmd); }

void permissions(std::string filename, bool read, bool write, bool execute) {
  if (read||write||execute) {
    mode_t mode = 0;
    if (read) mode = S_IRUSR;
    if (write) mode = (read) ? (mode | S_IWUSR) : S_IWUSR;
    if (execute) mode = (read || write) ? (mode | S_IXUSR) : S_IXUSR;
    chmod(filename.c_str(), mode);
  }
}

void makedirs(std::string path, bool make_last = false, mode_t mode = 0777) {
  std::vector<std::string> dirs;
  StringUtils::StringSplit(path, "/", dirs, true);

  std::string s = "";

  int end = (make_last) ? dirs.size() : dirs.size() - 1;
  for (int i = 0; i < end; i++) {
    s += dirs[i] + "/";
    try {
      makedir(s, mode);
    } catch (...) {
    }
  }
}

std::string join(std::vector<std::string> vec_i, mode_t i, bool makeDir, bool isFile) {
  if (vec_i.size() > 0) {
    std::string s = "";
    for (auto it = vec_i.begin(); it != vec_i.end(); ++it) {
      if (it->size() == 0)
        continue;
      else if (it->substr(0, 1).compare("/") == 0) {
        s = *it;  // If it starts with a "/" then its a root path.
      } else {
        s = s + *it;  // It gets appended.
      }

      // If we are not at the end, then add a slash.
      if ((it + 1) != vec_i.end()) {
        s += "/";
      }
    }

    if (makeDir) {
      makedirs(s, !isFile, i);
    }
    return s;
  }
  throw INJECTION_EXCEPTION_("Empty directory list passed to join");
}

static int info_callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {
  std::string name(info->dlpi_name);
  if (name.empty()) return 0;
  unsigned long add(info->dlpi_addr);
  libData* x = static_cast<libData*>(data);
  x->libs.push_back(DistUtils::getLibInfo(name, add));
  return 0;
}

void getAllLinkedLibraryData(libData* data) { dl_iterate_phdr(info_callback, data); }

void* loadLibrary(std::string name) {
  if (name.empty()) {
    throw INJECTION_EXCEPTION("File Name %s is invalid", name.c_str());
  }
  void* dllib = dlopen(name.c_str(), RTLD_NOW);

  return dllib;
}

registrationCallBack searchLibrary(void* dylib, std::string packageName) {
  void* callback = dlsym(dylib, packageName.c_str());
  if (callback != nullptr) {
    return ((registrationCallBack)callback);
  }
  throw INJECTION_EXCEPTION("Library Registration symbol not found for package %s", packageName.c_str());
}

bool fileExists(std::string filename) {
  struct stat info;
  return stat(filename.c_str(), &info) == 0;
}

std::string getEnvironmentVariable(std::string name, std::string def) {
  const char* val = std::getenv(name.c_str());
  if (val) {
    return val;
  }
  return def;
}

// Really trying to not need boost -- can get rid of this in C++17 (14
// is we use std::experimental and replace with std::filesystem. )
std::vector<std::string> listFilesInDirectory(std::string directory) {
  std::vector<std::string> res;

  DIR* dir;
  struct dirent* ent;
  if ((dir = opendir(directory.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      res.push_back(ent->d_name);
    }
    closedir(dir);
    return res;

  } else {
    throw INJECTION_EXCEPTION("Error Listing files in %s: Could not open directory", directory.c_str());
  }
}

}  // namespace DistUtils
}  // namespace VnV
