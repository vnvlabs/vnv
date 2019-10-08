
#include <sstream>
#include "vv-utils.h"

int VnV::StringSplit(const std::string &s,
                    const char *delim,
                    std::vector< std::string > &result )
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    result.clear();
    while ( std::getline( ss,item,delim[0]) )
    {
        if (!item.empty())
            result.push_back( item );
    }
    return 1;
}
  

std::string VnV::getFileExtension(const std::string& fileName)
{
      if (fileName.find_last_of(".") != std::string::npos)
        return fileName.substr(fileName.find_last_of(".")+1);
      return "";
}  
