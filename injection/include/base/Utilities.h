
/**
  @file Utilities.h
**/

#ifndef HEADER_UTILS_VV
#define HEADER_UTILS_VV
/**
 * @file vv-utils.h
 */
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "json-schema.hpp"
//#include "base/InjectionPoint.h"
namespace VnV {
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

namespace ProvenanceUtils {

std::string timeToString(std::string format = "%Y-%m-%d %H-%M-%S");

std::string cmdLineToString(int argc, char** argv);
}  // namespace ProvenanceUtils

namespace StringUtils {

std::string escapeQuotes(std::string str, bool escapeFullString);

std::string metaDataToJsonString(
    const std::map<std::string, std::string>& metadata);

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

std::string toString(std::vector<std::size_t> vector);

}  // namespace StringUtils

std::vector<std::pair<std::string, std::string>> bfs(
    std::map<std::string, std::map<std::string, std::string>>& m,
    std::string start, std::string end);

namespace JsonUtilities {

enum class CreateType { Object, Array, String, Float, Integer };
nlohmann::json& getOrCreate(nlohmann::json& parent, std::string key,
                            CreateType type = CreateType::Object);

nlohmann::json load(std::string s);

bool validate(const nlohmann::json& obj, const nlohmann::json& schema);

}  // namespace JsonUtilities

namespace MapUtilities {

template <typename Key, typename Value>
std::vector<Key> extractKeys(std::map<Key, Value> const& input) {
  std::vector<Key> res;
  res.reserve(input.size());
  for (auto const& el : input) res.push_back(el.first);
  return res;
}

template <typename Key, typename Value>
const Value& maxValue(std::map<Key, Value> const& input) {
  auto const& m = input.end();
  for (auto& el : input) {
    if (m == input.end() || el.second > m.second) {
      m = el;
    }
  }
  return m.second;
}

}  // namespace MapUtilities

namespace VariadicUtils {
NTV UnwrapVariadicArgs(va_list argp);
NTV UnwrapVariadicArgs(va_list argp, int count);
}  // namespace VariadicUtils
/**
 * @brief getFileExtension
 * @param fileName
 * @return
 */
std::string getFileExtension(const std::string& fileName);

namespace TimeUtils {

/**
 * @brief timeToISOString
 * @param t
 * @return
 *
 * Return the time as ISO string.
 */
std::string timeToISOString(time_t* t);

/**
 * @brief timeForFile
 * @param filename
 * @return
 *
 * Return the last modified time for a file using ISO String format
 */
std::string timeForFile(std::string filename);

std::string timestamp();


}  // namespace TimeUtils

namespace HashUtils {

std::size_t vectorHash(std::vector<int> const& vec);
}

namespace Log {
    void up();
    void down();
    std::ostream& log();
}

}  // namespace VnV



#endif
