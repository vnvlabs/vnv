#include "interfaces/points/Injection.h"
#include "base/stores/InjectionPointStore.h"
#include "base/Runtime.h"

void VnV::CppInjection::UnwrapParameterPack(NTV& /*m*/) {}

void VnV::defaultCallBack(
    VnV_Comm /*comm*/, std::map<std::string, VnVParameter>& /*params*/,
    IOutputEngine* /*engine*/, InjectionPointType /*type*/,
    std::string /*stageId*/) {}

void VnV::CppInjection::BeginLoop(VnV_Comm comm, const char* package,
                                  const char* id,
                                  const DataCallback& callback,
                                  NTV& map) {
  VnV::RunTime::instance().injectionPoint_begin(comm, package, id, callback,
                                                map);
}

void VnV::CppInjection::BeginPoint(VnV_Comm comm, const char* package,
                                   const char* id,
                                   const DataCallback& callback,
                                   NTV& map) {
  VnV::RunTime::instance().injectionPoint(comm, package, id, callback, map);
}

bool VnV::CppInjection::EndLoop(const char* package, const char* id) {
  VnV::RunTime::instance().injectionPoint_end(package, id);
  return true;
}
void VnV::CppInjection::IterLoop(const char* package, const char* id,
                                 const char* iterId) {
  VnV::RunTime::instance().injectionPoint_iter(package, id, iterId);
}

void VnV::CppInjection::RegisterInjectionPoint(const char* package, const char* id,std::string json) {
  VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint( package, id, json);
}

