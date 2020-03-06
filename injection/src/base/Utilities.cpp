
/** @file Utilities.cpp **/

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <xxhash.h>
#include <type_traits>

#include "base/Utilities.h"
#include "c-interfaces/Logging.h"

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

std::string VnV::hashfile( std::string filename) {

   std::ifstream fp(filename);
   if (! fp.good()) {
      VnV_Warn( "Could not open file %s", filename.c_str());
      return "<unknown>";
   }
   /* create a hash state */
   XXH64_state_t* const state = XXH64_createState();
   if (state==nullptr) abort();

   /* Initialize state with selected seed */
   XXH64_hash_t const seed = 0;   /* or any other value */
   if (XXH64_reset(state, seed) == XXH_ERROR) abort();

   /* Feed the state with input data, any size, any number of times */
   std::vector<char> buffer(1024,0);
   while (!fp.eof()) {
       fp.read(buffer.data(),buffer.size());
       std::streamsize dataSize = fp.gcount();
       if (XXH64_update(state, buffer.data(), dataSize) == XXH_ERROR) abort();
   }

   /* Get the hash */
   XXH64_hash_t const hash = XXH64_digest(state);
   XXH64_freeState(state);

   std::stringstream oss;
   oss << std::hex << std::setw(8) << std::setfill('0') << hash;
   return oss.str();

   return std::to_string(hash);

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
        throw "From not in graph";
    } else {
        bool yes = false;
        for (auto &t : m ) {
            if (t.second.find(end) != t.second.end() ) {
                yes = true;
                break;
            }
        }
        if (!yes) {
            throw "End node is not in the graph";
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
    throw "No Path";
}
