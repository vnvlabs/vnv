#ifndef HEADER_UTILS_VV
#define HEADER_UTILS_VV

#include <string>
#include <vector>

namespace VnV {

   int StringSplit(const std::string &s,const char *delim, std::vector< std::string > &result );
  
   std::string getFileExtension(const std::string& fileName);

  
}
#endif
