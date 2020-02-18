#include <sys/stat.h>
#include <unistd.h>
#include <link.h>
#include "base/DistUtils.h"
#include "base/Utilities.h"

#include "c-interfaces/RunTime.h"
#include "c-interfaces/Logging.h"
using nlohmann::json;
namespace VnV {
  namespace DistUtils {


json getLibInfo(std::string filepath, unsigned long add) {

    struct stat result;
    stat(filepath.c_str(),&result);
    json libJson;
    libJson["name"] = getAbsolutePath(filepath);
    libJson["add"] = add;
    libJson["hash"] = hashfile(filepath);
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

char* getCurrentDirectory() {
   return get_current_dir_name();
}

std::string getAbsolutePath(std::string filename) {
    char* x = realpath(filename.c_str(),nullptr);
    if (x != nullptr) {
        std::string path(x);
        free(x);
        return path;
    }
    return filename;
}

static int info_callback(struct dl_phdr_info* info, size_t /*size*/, void* data) {
  std::string name(info->dlpi_name);
  if (name.empty()) return 0;
  unsigned long add(info->dlpi_addr);
  libData* x = static_cast<libData*>(data);
  x->libs.push_back(DistUtils::getLibInfo(name,add));
  return 0;
}

void getAllLinkedLibraryData(libData *data) {
   dl_iterate_phdr(info_callback, data);
}

void * loadLibrary(std::string name) {
    if ( name.empty()) {
        throw "File Name invalid";
    }
    void* dllib = dlopen(name.c_str(), RTLD_NOW);

    if (dllib == nullptr) {
        throw "Could not open shared library";
    }
    return dllib;
}

bool searchLibrary(void *dylib, std::string packageName) {

    bool ret = false;
    std::string s = VNV_GET_REGISTRATION + packageName;
    auto a = VnV_BeginStage("Searching for VnV Registration Callback with name %s", s.c_str());

    void* callback = dlsym(dylib,s.c_str() );
    if ( callback != nullptr ) {
       ((registrationCallBack) callback)();
       ret = true;
       VnV_Debug("Found it");
    } else {
       ret = false;
    }
    VnV_EndStage(a);
    return ret;
}

bool searchLibrary(std::string name,  std::set<std::string> &packageNames) {

    void * dylib = loadLibrary(name);
    for (auto it : packageNames ) {
       searchLibrary(dylib,it);
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
 void callAllLibraryRegistrationFunctions(std::map<std::string,std::string> packageNames) {
    std::set<std::string> linked;
    for (auto it : packageNames) {
       linked.insert(it.first);
    }
    dl_iterate_phdr(load_callback, &packageNames);
}


} //namespace Dynamic
} //namespace VnV
