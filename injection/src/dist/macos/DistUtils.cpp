#include "base/DistUtils.h"

#include <dirent.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "c-interfaces/RunTime.h"
#include "json-schema.hpp"

using nlohmann::json;
namespace VnV {
namespace DistUtils {

json getLibInfo(std::string filepath, unsigned long add) {
  struct stat result;
  stat(filepath.c_str(), &result);
  json libJson;
  libJson["name"] = getAbsolutePath(filepath);
  libJson["add"] = add;
  libJson["st_dev"] = result.st_dev;
  libJson["st_gid"] = result.st_gid;
  libJson["st_ino"] = result.st_ino;
  libJson["st_uid"] = result.st_uid;
  libJson["st_mode"] = result.st_mode;
  libJson["st_rdev"] = result.st_rdev;
  libJson["st_size"] = result.st_size;
  libJson["st_nlink"] = result.st_nlink;
  libJson["st_blocks"] = result.st_blocks;
  libJson["st_blksize"] = result.st_blksize;
  libJson["st_atim_sec"] = result.st_atimespec.tv_sec;
  libJson["st_atim_nsec"] = result.st_atimespec.tv_nsec;
  libJson["st_ctim_sec"] = result.st_ctimespec.tv_sec;
  libJson["st_ctim_nsec"] = result.st_ctimespec.tv_nsec;
  libJson["st_mtim_sec"] = result.st_mtimespec.tv_sec;
  libJson["st_mtim_nsec"] = result.st_mtimespec.tv_nsec;
  return libJson;
}

char* getCurrentDirectory() { return getcwd(NULL, 0); }

std::string getAbsolutePath(std::string filename) {
  char* x = realpath(filename.c_str(), nullptr);
  if (x != nullptr) {
    std::string path(x);
    free(x);
    return path;
  }
  return filename;
}

void getAllLinkedLibraryData(libData* data) {
  uint32_t count = _dyld_image_count();
  for (uint32_t i = 0; i < count; i++) {
    const char* img_name = _dyld_get_image_name(i);
    std::string name(img_name);
    if (name.empty()) return;
    intptr_t img_addr = _dyld_get_image_vmaddr_slide(i);
    unsigned long add(img_addr);
    libData* x = static_cast<libData*>(data);
    x->libs.push_back(DistUtils::getLibInfo(name, add));
  }
}

void* loadLibrary(std::string name) {
  if (name.empty()) {
    throw VnVExceptionBase("File Name invalid");
  }
  void* dllib = dlopen(name.c_str(), RTLD_NOW);

  if (dllib == nullptr) {
    throw VnVExceptionBase("Could not open shared library");
  }
  return dllib;
}

registrationCallBack searchLibrary(void* dylib, std::string packageName) {
  std::string s = VNV_GET_REGISTRATION + packageName;
  void* callback = dlsym(dylib, s.c_str());
  if (callback != nullptr) {
    return ((registrationCallBack)callback);
  }
  throw VnVExceptionBase("Library Registration Symbol not found");
}

bool searchLibrary(std::string name, std::set<std::string>& packageNames) {
  void* dylib = loadLibrary(name);
  for (auto it : packageNames) {
    searchLibrary(dylib, it);
  }
  dlclose(dylib);
  return false;
}

// Not sure if works on mac -- please fix and commit if not compile.
std::string getEnvironmentVariable(std::string name) {
  std::string s = std::getenv(name.c_str());
  return s;
}

bool makedir(std::string filename, mode_t mode) {
  return mkdir(filename.c_str(), mode) == 0;
}

bool mv(std::string oldFileName, std::string newFileName) {
  return 0 == std::rename(oldFileName.c_str(), newFileName.c_str());
}

std::string join(std::vector<std::string> vector, mode_t i, bool makeDir) {
  if (vector.size() > 0) {
    std::string s = "";
    for (auto it : vector) {
      s = s + it + "/";
      if (makeDir) {
        struct stat sb;
        if (stat(s.c_str(), &sb) == 0) {
          if (S_ISDIR(sb.st_mode)) {
            continue;
          } else {
            throw VnVExceptionBase(
                "Cannot create directory as file with that name exists");
          }
        } else if (!makedir(s, i)) {
          throw VnV::VnVExceptionBase("Cannot make directory");
        }
      }
    }
    return s;
  }
  throw VnV::VnVExceptionBase("Empty directory list");
}

// Really trying to not need boost -- can get rid of this in C++17 (14
// is we use std::experimental and replace with std::filesystem. )
// Untested in macos;
std::vector<std::string> listFilesInDirectory(std::string directory) {
  std::vector<std::string> res;

  DIR* dir;
  struct dirent* ent;
  if ((dir = opendir(directory.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      res.push_back(ent->d_name);
    }
    closedir(dir);
  } else {
    throw VnVExceptionBase("Could not open directory");
  }
}

void callAllLibraryRegistrationFunctions(
    std::map<std::string, std::string> packageNames) {
  std::set<std::string> linked;
  for (auto it : packageNames) {
    linked.insert(it.first);
  }
  uint32_t count = _dyld_image_count();
  for (uint32_t i = 0; i < count; i++) {
    const char* img_name = _dyld_get_image_name(i);
    std::string name(img_name);
    try {
      searchLibrary(name, linked);
    } catch (...) {
      // VnV_Error("Could not load Shared Library %s", name.c_str());
      return;
    }
  }
}
}  // namespace DistUtils
}  // namespace VnV
