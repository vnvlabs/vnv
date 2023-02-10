

/**
  @file DistUtils.h
**/

#ifndef _VV_DYNAMIC_H
#define _VV_DYNAMIC_H
#include <set>

#include "base/LibraryInfo.h"
#include "base/Utilities.h"
#include "json-schema.hpp"

using nlohmann::json;

typedef void (*registrationCallBack)();

namespace VnV {
namespace DistUtils {

void* loadLibrary(std::string libraryPath);
registrationCallBack searchLibrary(void* dllib, std::string packageName);
/**
 * Get "stat" information for the file. Here add is the "address".
 **/
libInfo getLibInfo(std::string filepath, unsigned long add);

/**
 * @brief get the current working directory.
 * @return
 */
char* getCurrentDirectory();

class LockFile;
void initialize_lock(LockFile* lockfile);
void lock_file(LockFile* lockfile);
void unlock_file(LockFile* lockfile);
void close_file(LockFile* lockfile);

bool fileExists(std::string filename);

class LockFile {
 public:
  int fd;
  std::string fname;
  LockFile(std::string filename) : fname(filename) { initialize_lock(this); }

  void lock() { lock_file(this); }
  void unlock() { unlock_file(this); }
  void close() { close_file(this); }
};

bool fileEquals(std::string f1, std::string f2);
bool fileInDirectory(std::string file, std::string directory);

/**
 * Convert a relative filename to an absolute.
 */
std::string getAbsolutePath(std::string realativeFileName);
/**
 * @brief getLinkedLibraries
 * @param callBack
 * @param data
 *
 * Function that iterates over all linked libries and extract library data about
 * them.
 */
void getAllLinkedLibraryData(libData* data);

/**
 * Make the directory with the given mode.
 * @param filename
 * @param mode
 * @return
 */
bool makedir(std::string filename, mode_t mode);

bool mv(std::string oldFileName, std::string newFilename);
bool cp(std::string oldFileName, std::string newFilename);
bool ln(std::string oldFileName, std::string newFilename, bool hard = false);

class VnVProcess {
 public:
  virtual void wait() = 0;
  virtual bool running() = 0;
  virtual int getExitStatus() = 0;
  virtual void cancel() = 0;
  virtual std::string getStdout() = 0;
  virtual std::string getStdError() = 0;

  virtual ~VnVProcess() {}
};
std::shared_ptr<VnVProcess> exec(std::string cmd);

std::string getEnvironmentVariable(std::string val, std::string def = "");

std::string getTempFolder();
std::string getTempFile(std::string code = StringUtils::random(10), std::string ext = ".sh");

void permissions(std::string fname, bool read, bool write, bool execute);

/**
 * Make the nested directories with the given mode.
 * @param filename
 * @param mode
 * @return
 */
std::string join(std::vector<std::string> vector, mode_t i, bool makeDirs, bool isFile = true);

/**
 * List Files in Directory
 */
std::vector<std::string> listFilesInDirectory(std::string directory);

}  // namespace DistUtils
}  // namespace VnV

#endif  // VV_DYNAMIC_H
