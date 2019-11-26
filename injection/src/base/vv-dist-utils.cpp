#include "vv-dist-utils.h"
#include "vv-utils.h"
#include <sys/stat.h>
#include <unistd.h>


using nlohmann::json;
namespace VnV {
  namespace DistUtils {


json getLibInfo(std::string filepath, unsigned long add) {

    struct stat result;
    stat(filepath.c_str(),&result);
    json libJson;
    libJson["name"] = filepath;
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
#if defined(__APPLE__)
    libJson["st_atim_sec"] = result.st_atimespec.tv_sec;
    libJson["st_atim_nsec"] = result.st_atimespec.tv_nsec;
    libJson["st_ctim_sec"] = result.st_ctimespec.tv_sec;
    libJson["st_ctim_nsec"] = result.st_ctimespec.tv_nsec;
    libJson["st_mtim_sec"] = result.st_mtimespec.tv_sec;
    libJson["st_mtim_nsec"] = result.st_mtimespec.tv_nsec;
#else /* defined(__APPLE__) */
    libJson["st_atim_sec"] = result.st_atim.tv_sec;
    libJson["st_atim_nsec"] = result.st_atim.tv_nsec;
    libJson["st_ctim_sec"] = result.st_ctim.tv_sec;
    libJson["st_ctim_nsec"] = result.st_ctim.tv_nsec;
    libJson["st_mtim_sec"] = result.st_mtim.tv_sec;
    libJson["st_mtim_nsec"] = result.st_mtim.tv_nsec;
#endif /* defined(__APPLE__) */
    return libJson;
}

char* getCurrentDirectory() {
# if defined(__APPLE__)
    return getcwd(NULL,0);
# else
   return get_current_dir_name();
#endif
}

#if defined(__APPLE__)
struct dl_phdr_info {
  unsigned long dlpi_addr;
  char *dlpi_name;
};
#endif

typedef int (*dynamicCallBack)(struct dl_phdr_info *info, size_t size, void *data);

void iterateLinkedLibraries(dynamicCallBack callBack, void* data) {

#if defined (__APPLE__)
   VnV_Error("Get Linked Libraries Not implemented for MacOs");
#else
   dl_iterate_phdr(callBack, data);
#endif
}

} //namespace Dynamic
} //namespace VnV
