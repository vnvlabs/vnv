﻿

/**
  @file DistUtils.h
**/

#ifndef _VV_DYNAMIC_H
#define _VV_DYNAMIC_H
#include <set>

#include "base/LibraryInfo.h"
#include "json-schema.hpp"

using nlohmann::json;

typedef void (*registrationCallback)();

namespace VnV {
namespace DistUtils {

void* loadLibrary(std::string libraryPath);
registrationCallback searchLibrary(void* dllib, std::string packageName);
/**
 * Get "stat" information for the file. Here add is the "address".
 **/
libInfo getLibInfo(std::string filepath, unsigned long add);

/**
 * @brief get the current working directory.
 * @return
 */
char* getCurrentDirectory();

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

void callAllLibraryRegistrationFunctions(
    std::map<std::string, std::string> packageNames);

/**
 * Make the directory with the given mode.
 * @param filename
 * @param mode
 * @return
 */
bool makedir(std::string filename, mode_t mode);

bool mv(std::string oldFileName, std::string newFilename);

std::string getEnvironmentVariable(std::string val);

/**
 * Make the nested directories with the given mode.
 * @param filename
 * @param mode
 * @return
 */
std::string join(std::vector<std::string> vector, mode_t i, bool makeDirs);

/**
 * List Files in Directory
 */
std::vector<std::string> listFilesInDirectory(std::string directory);

}  // namespace DistUtils
}  // namespace VnV

#endif  // VV_DYNAMIC_H
