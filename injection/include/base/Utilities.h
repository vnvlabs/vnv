
/**
  @file Utilities.h
**/

#ifndef HEADER_UTILS_VV
#define HEADER_UTILS_VV
/**
 * @file vv-utils.h
 */
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "json-schema.hpp"
//#include "base/InjectionPoint.h"
namespace VnV {
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

namespace StringUtils {

std::string escapeQuotes(std::string str, bool escapeFullString);

void ltrim(std::string& s);

// trim from end (in place)
void rtrim(std::string& s);

// trim from both ends (in place)
void trim(std::string& s);

// trim from start (copying)
std::string ltrim_copy(std::string s);
// trim from end (copying)
std::string rtrim_copy(std::string s);

// trim from both ends (copying)
std::string trim_copy(std::string s);

std::string squash_copy(std::string s);

std::string squash(std::string& s);

std::string get_type(std::string s);

std::vector<std::string> process_variadic(const char* args);

std::pair<std::string, std::string> splitCppArgString(std::string str_);

std::map<std::string, std::string> variadicProcess(const char* mess);

std::string getIndent(int level, std::string space = "\t");

long long simpleHash(const std::string& str);

bool balancedParenthesis(std::string expr);

/**
 * @brief StringSplit
 * @param s
 * @param delim
 * @param result
 * @return
 */
int StringSplit(const std::string& s, const char* delim,
                std::vector<std::string>& result);

template <typename ContainerT, typename PredicateT>
void erase_if(ContainerT& items, const PredicateT& predicate) {
  for (auto it = items.begin(); it != items.end();) {
    if (predicate(*it))
      it = items.erase(it);
    else
      ++it;
  }
}

}  // namespace StringUtils

std::vector<std::pair<std::string, std::string>> bfs(
    std::map<std::string, std::map<std::string, std::string>>& m,
    std::string start, std::string end);

namespace JsonUtilities {

enum class CreateType { Object, Array, String, Float, Integer };
nlohmann::json& getOrCreate(nlohmann::json& parent, std::string key,
                            CreateType type);

}  // namespace JsonUtilities

namespace VariadicUtils {
NTV UnwrapVariadicArgs(va_list argp);
}
/**
 * @brief getFileExtension
 * @param fileName
 * @return
 */
std::string getFileExtension(const std::string& fileName);

}  // namespace VnV
#endif
