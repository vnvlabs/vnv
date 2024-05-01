

/**
  @file DistUtils.h
**/

#ifndef _VV2_DYNAMIC_H
#define _VV2_DYNAMIC_H
#include <set>

#include "shared/LibraryInfo.h"
#include "shared/Utilities.h"
#include "json-schema.hpp"

using nlohmann::json;

typedef void (*registrationCallBack)();

namespace VnV {
namespace DistUtils {

void* loadLibrary(std::string libraryPath);
registrationCallBack searchLibrary(void* dllib, std::string packageName);



/**
 * @brief getLinkedLibraries
 * @param callBack
 * @param data
 *
 * Function that iterates over all linked libries and extract library data about
 * them.
 */
void getAllLinkedLibraryData(libData* data);


class VnVProcess {
 public:
  virtual void wait() = 0;
  virtual bool running() = 0;
  virtual int getExitStatus() = 0;
  virtual void cancel() = 0;
  virtual std::string getStdout() = 0;
  virtual std::string getStdError() = 0;

  virtual ~VnVProcess() {}
};
std::shared_ptr<VnVProcess> exec(std::string cmd);


}  // namespace DistUtils
}  // namespace VnV

#endif  // VV_DYNAMIC_H
