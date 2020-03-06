
/**
  @file Utilities.h
**/

#ifndef HEADER_UTILS_VV
#define HEADER_UTILS_VV
/**
 * @file vv-utils.h
 */
#include <string>
#include <vector>
#include "json-schema.hpp"

namespace VnV {

namespace StringUtils {
void ltrim(std::string &s) ;

// trim from end (in place)
void rtrim(std::string &s) ;

// trim from both ends (in place)
void trim(std::string &s) ;

// trim from start (copying)
std::string ltrim_copy(std::string s) ;
// trim from end (copying)
std::string rtrim_copy(std::string s);

// trim from both ends (copying)
std::string trim_copy(std::string s);

/**
 * @brief StringSplit
 * @param s
 * @param delim
 * @param result
 * @return
 */
int StringSplit(const std::string& s, const char* delim,
                std::vector<std::string>& result);
}

std::vector<std::pair<std::string,std::string>> bfs(std::map<std::string,std::map<std::string, std::string>> &m, std::string start, std::string end);

namespace JsonUtilities {

 enum class CreateType { Object, Array, String, Float, Integer };

 nlohmann::json& getOrCreate(nlohmann::json& parent, std::string key, CreateType type );



}


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
