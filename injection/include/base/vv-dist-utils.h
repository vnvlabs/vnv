
#ifndef _VV_DYNAMIC_H
#define _VV_DYNAMIC_H

#if !defined(__APPLE__)
#include <link.h>
#endif /* !defined(__APPLE__) */

#include "json-schema.hpp"
using nlohmann::json;

#if defined(__APPLE__)
struct dl_phdr_info {
  unsigned long dlpi_addr;
  char *dlpi_name;
};
#endif

namespace VnV {
  namespace DistUtils {


typedef int (*dynamicCallBack)(struct dl_phdr_info *info, size_t size, void *data);

/**
 * Get "stat" information for the file. Here add is the "address".
 **/
json getLibInfo(std::string filepath, unsigned long add);

/**
 * @brief get the current working directory.
 * @return
 */
char* getCurrentDirectory();

/**
 * Convert a relative filename to an absolute.
 */
std::string getAbsolutePath(std::string realativeFileName);
/**
 * @brief getLinkedLibraries
 * @param callBack
 * @param data
 *
 * Callback that iterates over all linked libraries.
 */
void iterateLinkedLibraries(dynamicCallBack callBack, void* data);

} //namespace Dynamic
} //namespace VnV

#endif //VV_DYNAMIC_H




