#include "interfaces/points/Injection.h"

#include "base/Runtime.h"
#include "base/stores/InjectionPointStore.h"

void VnV::CppInjection::UnwrapParameterPack(NTV& /*m*/) {}

void VnV::defaultCallBack(VnV_Comm /*comm*/, VnVParameterSet& /*params*/, IOutputEngine* /*engine*/,
                          InjectionPointType /*type*/, std::string /*stageId*/) {}

void VnV::CppInjection::BeginLoop(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                                  const char* fname, int line, const DataCallback& callback, NTV& map) {
  try {
    VnV::RunTime::instance().injectionPoint_begin(comm, package, id, pretty, fname, line, callback, map);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
  }
}

void VnV::CppInjection::BeginPoint(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                                   const char* fname, int line, const DataCallback& callback, NTV& map) {
  try {
    VnV::RunTime::instance().injectionPoint(comm, package, id, pretty, fname, line, callback, map);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
  }
}

bool VnV::CppInjection::EndLoop(const char* package, const char* id, const char* fname, int line) {
  try {
    VnV::RunTime::instance().injectionPoint_end(package, id, fname, line);
    return true;
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
    return true;
  }
}
void VnV::CppInjection::IterLoop(const char* package, const char* id, std::string iterId, const char* fname, int line) {
  try {
    VnV::RunTime::instance().injectionPoint_iter(package, id, iterId, fname, line);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
  }
}

void VnV::CppInjection::RegisterInjectionPoint(const char* package, const char* id, std::string json) {
  try {
    VnV::InjectionPointStore::instance().registerInjectionPoint(package, id, json);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
  }
}
