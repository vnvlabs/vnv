
/** @file vv-utils.cpp **/

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <xxhash.h>

#include "json-schema.hpp"
#include "base/Utilities.h"

#include "c-interfaces/Logging.h"

int VnV::StringSplit(const std::string& s, const char* delim,
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

