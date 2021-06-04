
/**
  @file DistUtils.cpp
**/

#include "base/DistUtils.h"

#include <link.h>
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
  libJson["st_atim_sec"] = result.st_atim.tv_sec;
  libJson["st_atim_nsec"] = result.st_atim.tv_nsec;
  libJson["st_ctim_sec"] = result.st_ctim.tv_sec;
  libJson["st_ctim_nsec"] = result.st_ctim.tv_nsec;
  libJson["st_mtim_sec"] = result.st_mtim.tv_sec;
  libJson["st_mtim_nsec"] = result.st_mtim.tv_nsec;
  return libJson;
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

bool makedir(std::string filename, mode_t mode ) {
  return mkdir(filename.c_str(), mode) == 0;
}



std::string join(std::vector<std::string> vector, mode_t i, bool makeDir) {
  if (vector.size() > 0 ) {
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
                  "Cannot create directory as file with that name exists (%s)", s.c_str());
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

}  // namespace DistUtils
}  // namespace VnV
