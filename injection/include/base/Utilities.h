#ifndef HEADER_UTILS_VV
#define HEADER_UTILS_VV
/**
 * @file vv-utils.h
 */
#include <string>
#include <vector>

namespace VnV {
/**
 * @brief StringSplit
 * @param s
 * @param delim
 * @param result
 * @return
 */
int StringSplit(const std::string& s, const char* delim,
                std::vector<std::string>& result);

/**
 * @brief getFileExtension
 * @param fileName
 * @return
 */
std::string getFileExtension(const std::string& fileName);

/**
 * @brief MD5
 * @param filename
 * @return
 *
 * @todo A really bad hash algo for testing.
 */
std::string hashfile(std::string filename);


}  // namespace VnV
#endif
