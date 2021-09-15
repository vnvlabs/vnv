
/**
  @file DistUtils.cpp
**/

#include "base/DistUtils.h"

#include <dirent.h>
#include <link.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
#include "c-interfaces/RunTime.h"
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

std::string getAbsolutePath(std::string filename) {
  char* x = realpath(filename.c_str(), nullptr);
  if (x != nullptr) {
    std::string path(x);
    free(x);
    return path;
  }
  return filename;
}

bool makedir(std::string filename, mode_t mode) {
  return mkdir(filename.c_str(), mode) == 0;
}

bool mv(std::string oldFileName, std::string newFileName) {
  return 0 == std::rename(oldFileName.c_str(), newFileName.c_str());
}

std::string join(std::vector<std::string> vec, mode_t i, bool makeDir) {
  if (vec.size() > 0) {
    std::string s = "";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
      s = s + *it + (((it + 1) == vec.end()) ? "" : "/");

      if (makeDir) {
        struct stat sb;
        if (stat(s.c_str(), &sb) == 0) {
          if (S_ISDIR(sb.st_mode)) {
            continue;
          } else {
            throw VnVExceptionBase(
                "Cannot create directory as file with that name exists (%s)",
                s.c_str());
          }
        } else if (!makedir(s, i)) {
          throw VnV::VnVExceptionBase("Cannot make directory %s", s.c_str());
        }
      }
    }
    return s;
  }
  throw VnV::VnVExceptionBase("Empty directory list");
}

static int info_callback(struct dl_phdr_info* info, size_t /*size*/,
                         void* data) {
  std::string name(info->dlpi_name);
  if (name.empty()) return 0;
  unsigned long add(info->dlpi_addr);
  libData* x = static_cast<libData*>(data);
  x->libs.push_back(DistUtils::getLibInfo(name, add));
  return 0;
}

void getAllLinkedLibraryData(libData* data) {
  dl_iterate_phdr(info_callback, data);
}

void* loadLibrary(std::string name) {
  if (name.empty()) {
    throw VnVExceptionBase("File Name invalid");
  }
  void* dllib = dlopen(name.c_str(), RTLD_NOW);

  if (dllib == nullptr) {
    std::cout << "Could not load library " << dlerror() << std::endl;
    throw VnVExceptionBase("Could not open shared library");
  }
  return dllib;
}

registrationCallBack searchLibrary(void* dylib, std::string packageName) {
  bool ret = false;
  std::string s = VNV_GET_REGISTRATION + packageName;

  void* callback = dlsym(dylib, s.c_str());
  if (callback != nullptr) {
    return ((registrationCallBack)callback);
  }
  throw VnVExceptionBase("Library Registration symbol not found");
}

bool searchLibrary(std::string name, std::set<std::string>& packageNames) {
  void* dylib = loadLibrary(name);
  for (auto it : packageNames) {
    if (searchLibrary(dylib, it)) {
      dlclose(dylib);
      return true;
    }
  }
  dlclose(dylib);
  return false;
}

int load_callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {
  std::string name(info->dlpi_name);  // Library name
  try {
    searchLibrary(name, *((std::set<std::string>*)data));
  } catch (...) {
    // VnV_Error("Could not load Shared Library %s", name.c_str());
  }
  return 0;
}
// packageName -> filename.
void callAllLibraryRegistrationFunctions(
    std::map<std::string, std::string> packageNames) {
  std::set<std::string> linked;
  for (auto it : packageNames) {
    linked.insert(it.first);
  }
  dl_iterate_phdr(load_callback, &linked);
}

std::string getEnvironmentVariable(std::string name) {
  std::string s = std::getenv(name.c_str());
  return s;
}

// Really trying to not need boost -- can get rid of this in C++17 (14
// is we use std::experimental and replace with std::filesystem. )
std::vector<std::string> listFilesInDirectory(std::string directory) {
  std::vector<std::string> res;

  DIR* dir;
  struct dirent* ent;
  VnV_Debug(VNVPACKAGENAME, "Openning directory %ld", directory.size());
  if ((dir = opendir(directory.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      res.push_back(ent->d_name);
    }
    closedir(dir);
    return res;
  } else {
    throw VnVExceptionBase("Could not open directory");
  }
}

}  // namespace DistUtils
}  // namespace VnV
