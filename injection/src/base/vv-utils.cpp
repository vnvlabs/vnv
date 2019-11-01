
/** @file vv-utils.cpp **/

#include "vv-utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

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
   std::stringstream oss;
   if ( !fp.is_open()) {
       return "";
   }
   uint32_t magic = 5381;
   char c;
   while( fp.get(c)) {
       magic = ((magic << 5) + magic) + c;
   }
   oss << std::hex << std::setw(8) << std::setfill('0') << magic;
   return oss.str();
}
