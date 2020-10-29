
/** @file Utilities.cpp **/

#include "base/Utilities.h"

#include <time.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <type_traits>
//#include "base/InjectionPoint.h"
#include <sys/stat.h>
#include <time.h>

#include <list>
#include <queue>
#include <set>

#include "base/exceptions.h"
#include "c-interfaces/PackageName.h"

using nlohmann::json;

std::string VnV::ProvenanceUtils::timeToString(std::string format) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, format.c_str());
  return oss.str();
}

std::string VnV::ProvenanceUtils::cmdLineToString(int argc, char** argv) {
  std::ostringstream commandline;
  commandline << argv[0];
  for (int i = 1; i < argc; i++) {
    commandline << " " << std::string(argv[i]);
  }
  return commandline.str();
}

std::string VnV::StringUtils::escapeQuotes(std::string str,
                                           bool escapeFullString) {
  std::ostringstream oss;
  if (escapeFullString) oss << "\"";
  for (std::size_t i = 0; i < str.size(); i++) {
    switch (str[i]) {
    case '\a':
      oss << "\\a";
      break;
    case '\b':
      oss << "\\b";
      break;
    case '\t':
      oss << "\\t";
      break;
    case '\n':
      oss << "\\n";
      break;
    case '\v':
      oss << "\\v";
      break;
    case '\f':
      oss << "\\f";
      break;
    case '\r':
      oss << "\\r";
      break;
    case '\"':
      oss << "\\\"";
      break;
    case '\'':
      oss << "\\\'";
      break;
    case '\?':
      oss << "\\\?";
      break;
    case '\\':
      oss << "\\\\";
      break;
    default:
      oss << str[i];
    }
  }
  if (escapeFullString) oss << "\"";
  return oss.str();
}

// trim from start (in place)
void VnV::StringUtils::ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
void VnV::StringUtils::rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
void VnV::StringUtils::trim(std::string& s) {
  ltrim(s);
  rtrim(s);
}

// trim from start (copying)
std::string VnV::StringUtils::ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// trim from end (copying)
std::string VnV::StringUtils::rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// trim from both ends (copying)
std::string VnV::StringUtils::trim_copy(std::string s) {
  trim(s);
  return s;
}

std::string VnV::StringUtils::squash(std::string& str) {
  std::string::iterator end_pos = std::remove(str.begin(), str.end(), ' ');
  str.erase(end_pos, str.end());
  return str;
}

std::string VnV::StringUtils::squash_copy(std::string s) {
  squash(s);
  return s;
}

std::string VnV::StringUtils::get_type(std::string s) {
  squash(s);
  if (s.empty()) {
    return s;
  } else if (s[0] == '(' && s[s.size() - 1] == ')') {
    return s.substr(1, s.size() - 1);
  } else {
    return s;
  }
}

int VnV::StringUtils::StringSplit(const std::string& s, const char* delim,
                                  std::vector<std::string>& result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  result.clear();
  while (std::getline(ss, item, delim[0])) {
    if (!item.empty()) result.push_back(item);
  }
  return 1;
}

nlohmann::json& VnV::JsonUtilities::getOrCreate(json& parent, std::string key,
                                                CreateType type) {
  if (!parent.contains(key)) {
    switch (type) {
    case CreateType::Object:
      parent[key] = json::object();
      break;
    case CreateType::Array:
      parent[key] = json::array();
      break;
    case CreateType::Float:
      parent[key] = 1.0;
      break;
    case CreateType::String:
      parent[key] = "";
      break;
    case CreateType::Integer:
      parent[key] = 0;
      break;
    }
  }
  return parent[key];
}

std::string VnV::getFileExtension(const std::string& fileName) {
  if (fileName.find_last_of(".") != std::string::npos)
    return fileName.substr(fileName.find_last_of(".") + 1);
  return "";
}

VnV::NTV VnV::VariadicUtils::UnwrapVariadicArgs(va_list argp) {
  VnV::NTV parameterSet;
  while (1) {
    std::string variableName = va_arg(argp, char*);
    if (variableName == VNV_END_PARAMETERS_S) {
      break;
    }
    void* variablePtr = va_arg(argp, void*);

    // add it to the list. In C, we do not have typeId, so we cannot get the
    // rtti info.
    // Just set it to void*.
    parameterSet.insert(
        std::make_pair(variableName, std::make_pair("void*", variablePtr)));
  }
  return parameterSet;
}

std::vector<std::pair<std::string, std::string>> VnV::bfs(
    std::map<std::string, std::map<std::string, std::string>>& m,
    std::string start, std::string end) {
  std::queue<std::string> queue({start});
  std::set<std::string> visited;
  std::map<std::string, std::string> parentMap;

  if (start == end) {
    return {};  // Nothing to be done.
  } else if (m.find(start) == m.end()) {
    throw VnVExceptionBase("From not in graph");
  } else {
    bool yes = false;
    for (auto& t : m) {
      if (t.second.find(end) != t.second.end()) {
        yes = true;
        break;
      }
    }
    if (!yes) {
      throw VnVExceptionBase("End node is not in the graph");
    }
  }
  while (!queue.empty()) {
    // Gets the first path in the queue
    std::string front = queue.front();
    queue.pop();

    if (front == end) {
      // Front is the name of this node. From front to to.
      // Result is m[front][to]
      // We are done. Plus (base case means we have at least one transform.
      auto it = parentMap.find(front);
      std::vector<std::pair<std::string, std::string>> result;
      while (it != parentMap.end()) {
        result.push_back(std::make_pair(front, m[it->second][front]));
        front = it->second;
        it = parentMap.find(it->second);
      }
      std::reverse(result.begin(), result.end());
      return result;
    }
    auto it = m.find(front);
    if (it != m.end()) {
      for (auto itt : it->second) {
        if (visited.find(itt.first) == visited.end()) {
          visited.insert(itt.first);
          queue.push(itt.first);
          parentMap[itt.first] = front;
        }
      }
    }
  }
  throw VnVExceptionBase("No Path");
}

std::map<std::string, std::string> VnV::StringUtils::variadicProcess(
    const char* mess) {
  std::map<std::string, std::string> res;
  std::vector<std::string> pmess = process_variadic(mess);
  // We need to support class types with commas in the name. To support that,
  // the simplest way is to require classes with a comma be wrapped up inside a
  // pair of parenthesis. So, we dont consider an entry finished until we
  // balanence the parenthesis
  std::size_t count = 0;
  std::string curr = "";
  while (count < pmess.size()) {
    curr += ((curr.empty()) ? "" : ",") + pmess[count++];
    if (balancedParenthesis(curr)) {
      std::pair<std::string, std::string> p = splitCppArgString(curr);
      res[squash_copy(p.second)] = squash_copy(p.first);
      curr = "";
    } else if (count == pmess.size()) {
      throw VnVExceptionBase("Unbalenced parenthesis in class name");
    }
  }
  return res;
}

std::pair<std::string, std::string> VnV::StringUtils::splitCppArgString(
    std::string str_) {
  std::string str = VnV::StringUtils::trim_copy(str_);
  std::size_t last = str.find_last_of("&*> ");
  return std::make_pair(str.substr(0, last + 1), str.substr(last + 1));
}

std::vector<std::string> VnV::StringUtils::process_variadic(const char* args) {
  std::string mess = args;
  std::vector<std::string> res;
  StringSplit(mess, ",", res);
  return res;
}

bool VnV::StringUtils::balancedParenthesis(std::string expr) {
  std::stack<char> s;
  char x;

  // Traversing the Expression
  for (std::size_t i = 0; i < expr.length(); i++) {
    if (expr[i] == '(' || expr[i] == '[' || expr[i] == '{') {
      // Push the element in the stack
      s.push(expr[i]);
      continue;
    }

    switch (expr[i]) {
    case ')':
      if (s.empty()) return false;
      // Store the top element in a
      x = s.top();
      s.pop();
      if (x == '{' || x == '[') return false;
      break;

    case '}':
      if (s.empty()) return false;
      // Store the top element in b
      x = s.top();
      s.pop();
      if (x == '(' || x == '[') return false;
      break;

    case ']':
      if (s.empty()) return false;
      // Store the top element in c
      x = s.top();
      s.pop();
      if (x == '(' || x == '{') return false;
      break;
    default:
      continue;
    }
  }

  // Check Empty Stack
  return (s.empty());
}

std::string VnV::StringUtils::getIndent(int level, std::string space) {
  std::string s = "";
  for (int i = 0; i < level; i++) {
    s += space;
  }
  return s;
}

long long VnV::StringUtils::simpleHash(const std::string& str) {
  const int p = 31;
  const int m = 1e9 + 9;
  long long hash_value = 0;
  long long p_pow = 1;
  for (char c : str) {
    hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
    p_pow = (p_pow * p) % m;
  }
  return hash_value;
}

std::string VnV::TimeUtils::timeToISOString(time_t* t) {
  char buf[80];
  std::strftime(buf, 80, "%Y-%m-%d-%H-%M-%S", std::gmtime(t));
  return buf;
}

std::string VnV::TimeUtils::timeForFile(std::string filename) {
  struct stat result;
  stat(filename.c_str(), &result);
  return timeToISOString(&result.st_mtime);
}

std::size_t VnV::HashUtils::vectorHash(std::vector<int> const& vec) {
  std::size_t seed = vec.size();
  for(auto& i : vec) {
    seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}
