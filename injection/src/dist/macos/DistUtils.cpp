#include "base/vv-dist-utils.h"
#include "base/vv-utils.h"
#include <sys/stat.h>
#include <unistd.h>

#include "json-schema.hpp"
#include "c-interfaces/logging-interface.h"

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
    libJson["st_atim_sec"] = result.st_atimespec.tv_sec;
    libJson["st_atim_nsec"] = result.st_atimespec.tv_nsec;
    libJson["st_ctim_sec"] = result.st_ctimespec.tv_sec;
    libJson["st_ctim_nsec"] = result.st_ctimespec.tv_nsec;
    libJson["st_mtim_sec"] = result.st_mtimespec.tv_sec;
    libJson["st_mtim_nsec"] = result.st_mtimespec.tv_nsec;
    return libJson;
}

char* getCurrentDirectory() {
    return getcwd(NULL,0);
}

std::string getAbsolutePath(std::string filename) {
    VnV_Warn("TODO MacOS function to convert to absoulte file path.");
    return filename;
}

void getAllLinkedLibraryData(libData *data) {
    VnV_Error("Get Linked Libraries Not implemented for MacOs");
}

} //namespace Dynamic
} //namespace VnV

