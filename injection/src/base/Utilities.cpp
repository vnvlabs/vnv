
/** @file Utilities.cpp **/

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <map>

#include "base/Utilities.h"
#include "c-interfaces/Logging.h"
#include "base/InjectionPoint.h"

using nlohmann::json;

// trim from start (in place)
void VnV::StringUtils::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
void VnV::StringUtils::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void VnV::StringUtils::trim(std::string &s) {
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


nlohmann::json& VnV::JsonUtilities::getOrCreate(json &parent, std::string key, CreateType type) {
    if (!parent.contains(key)) {
        switch (type) {
            case CreateType::Object : parent[key] = json::object(); break;
            case CreateType::Array : parent[key] = json::array(); break;
            case CreateType::Float : parent[key] = 1.0; break;
            case CreateType::String : parent[key] = ""; break;
            case CreateType::Integer : parent[key] = 0; break;
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

      //add it to the list. In C, we do not have typeId, so we cannot get the rtti info.
      // Just set it to void*.
      parameterSet.insert(std::make_pair(variableName, std::make_pair("void*", variablePtr)));

    }
    return parameterSet;
}

#include <set>
#include <list>
#include <queue>

std::vector<std::pair<std::string,std::string>> VnV::bfs(std::map<std::string,std::map<std::string, std::string>> &m, std::string start, std::string end) {
    std::queue<std::string> queue({start});
    std::set<std::string> visited;
    std::map<std::string,std::string> parentMap;

    if (start == end) {
        return {}; // Nothing to be done.
    } else if (m.find(start) == m.end()) {
        throw VnVExceptionBase("From not in graph");
    } else {
        bool yes = false;
        for (auto &t : m ) {
            if (t.second.find(end) != t.second.end() ) {
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
            std::vector<std::pair<std::string,std::string>> result;
            while (it != parentMap.end() ) {
                result.push_back(std::make_pair(front, m[it->second][front]));
                front = it->second;
                it = parentMap.find(it->second);
            }
            std::reverse(result.begin(),result.end());
            return result;
        }
        auto it = m.find(front);
        if ( it != m.end() ) {
            for (auto itt : it->second) {
                if (visited.find(itt.first) ==visited.end()) {
                    visited.insert(itt.first);
                    queue.push(itt.first);
                    parentMap[itt.first] = front;
                }
            }
        }
    }
    throw VnVExceptionBase("No Path");
}

std::map<std::string, std::string> VnV::StringUtils::variadicProcess(const char *mess) {
    std::map<std::string,std::string> res;
    std::vector<std::string> pmess = process_variadic(mess);
    for (auto it : pmess) {
        std::pair<std::string, std::string> p = splitCppArgString(it);
        res[trim_copy(p.second)] = trim_copy(p.first);
    }
    return res;
}

std::pair<std::string, std::string> VnV::StringUtils::splitCppArgString(std::string str_) {
    std::string str = VnV::StringUtils::trim_copy(str_);
    std::size_t last = str.find_last_of("&*> ");
    return std::make_pair(str.substr(0,last+1), str.substr(last+1));
}

std::vector<std::string> VnV::StringUtils::process_variadic(const char *args) {
    std::string mess = args;
    std::vector<std::string> res;
    StringSplit(mess,",",res);
    return res;
}

bool VnV::StringUtils::balancedParenthesis(std::string expr) {
    std::stack<char> s;
    char x;

    // Traversing the Expression
    for (int i=0; i<expr.length(); i++)
    {
        if (expr[i]=='('||expr[i]=='['||expr[i]=='{')
        {
            // Push the element in the stack
            s.push(expr[i]);
            continue;
        }

        // IF current current character is not opening
        // bracket, then it must be closing. So stack
        // cannot be empty at this point.
        if (s.empty())
            return false;

        switch (expr[i])
        {
        case ')':

            // Store the top element in a
            x = s.top();
            s.pop();
            if (x=='{' || x=='[')
                return false;
            break;

        case '}':

            // Store the top element in b
            x = s.top();
            s.pop();
            if (x=='(' || x=='[')
                return false;
            break;

        case ']':

            // Store the top element in c
            x = s.top();
            s.pop();
            if (x =='(' || x == '{')
                return false;
            break;
        }
    }

    // Check Empty Stack
    return (s.empty());
}
