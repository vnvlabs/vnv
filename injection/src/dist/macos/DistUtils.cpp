#include "base/DistUtils.h"
#include "base/Utilities.h"
#include <sys/stat.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>

#include "json-schema.hpp"
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
    libJson["st_atim_sec"] = result.st_atimespec.tv_sec;
    libJson["st_atim_nsec"] = result.st_atimespec.tv_nsec;
    libJson["st_ctim_sec"] = result.st_ctimespec.tv_sec;
    libJson["st_ctim_nsec"] = result.st_ctimespec.tv_nsec;
    libJson["st_mtim_sec"] = result.st_mtimespec.tv_sec;
    libJson["st_mtim_nsec"] = result.st_mtimespec.tv_nsec;
    return libJson;
}

char* getCurrentDirectory() {
    return getcwd(NULL, 0);
}

std::string getAbsolutePath(std::string filename) {
    char *x = realpath(filename.c_str(), nullptr);
    if (x != nullptr) {
        std::string path(x);
        free(x);
        return path;
    }
    return filename;
}

void getAllLinkedLibraryData(libData *data) {
    uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        const char *img_name = _dyld_get_image_name(i);
        std::string name(img_name);
        if (name.empty())
            return;
        intptr_t img_addr = _dyld_get_image_vmaddr_slide(i);
        unsigned long add(img_addr);
        libData *x = static_cast<libData*>(data);
        x->libs.push_back(DistUtils::getLibInfo(name, add));
    }
}

void* loadLibrary(std::string name) {
    if (name.empty()) {
        throw "File Name invalid";
    }
    void *dllib = dlopen(name.c_str(), RTLD_NOW);

    if (dllib == nullptr) {
        throw "Could not open shared library";
    }
    return dllib;
}

bool searchLibrary(void *dylib, std::string packageName) {
    bool ret = false;
    std::string s = VNV_GET_REGISTRATION + packageName;
    auto a = VnV_BeginStage(
            "Searching for VnV Registration Callback with name %s", s.c_str())
    ;

    void *callback = dlsym(dylib, s.c_str());
    if (callback != nullptr) {
        ((registrationCallBack) callback)();
        ret = true;
        VnV_Debug("Found it");
    } else {
        ret = false;
    }
    VnV_EndStage(a);
    return ret;
}

bool searchLibrary(std::string name, std::set<std::string> &packageNames) {
    void *dylib = loadLibrary(name);
    for (auto it : packageNames) {
        searchLibrary(dylib, it);
    }
    dlclose(dylib);
    return false;
}

void callAllLibraryRegistrationFunctions(
        std::map<std::string, std::string> packageNames) {
    std::set<std::string> linked;
    for (auto it : packageNames) {
        linked.insert(it.first);
    }
    uint32_t count = _dyld_image_count();
    for (uint32_t i = 0; i < count; i++) {
        const char *img_name = _dyld_get_image_name(i);
        std::string name(img_name);
        try {
            searchLibrary(name, linked);
        } catch (...) {
            // VnV_Error("Could not load Shared Library %s", name.c_str());
            return;
        }
    }
}
} //namespace Dynamic
} //namespace VnV

