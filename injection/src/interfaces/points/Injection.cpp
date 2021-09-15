#include "interfaces/points/Injection.h"

#include "base/Runtime.h"
#include "base/stores/InjectionPointStore.h"

void VnV::CppInjection::UnwrapParameterPack(NTV& /*m*/) {}

void VnV::defaultCallBack(VnV_Comm /*comm*/,
                          std::map<std::string, VnVParameter>& /*params*/,
                          IOutputEngine* /*engine*/,
                          InjectionPointType /*type*/,
                          std::string /*stageId*/) {}

void VnV::CppInjection::BeginLoop(VnV_Comm comm, const char* package,
                                  const char* id, const char* fname, int line,
                                  const DataCallback& callback, NTV& map) {
  VnV::RunTime::instance().injectionPoint_begin(comm, package, id, fname, line,
                                                callback, map);
}

void VnV::CppInjection::BeginPoint(VnV_Comm comm, const char* package,
                                   const char* id, const char* fname, int line,
                                   const DataCallback& callback, NTV& map) {
  VnV::RunTime::instance().injectionPoint(comm, package, id, fname, line,
                                          callback, map);
}

bool VnV::CppInjection::EndLoop(const char* package, const char* id,
                                const char* fname, int line) {
  VnV::RunTime::instance().injectionPoint_end(package, id, fname, line);
  return true;
}
void VnV::CppInjection::IterLoop(const char* package, const char* id,
                                 const char* iterId, const char* fname,
                                 int line) {
  VnV::RunTime::instance().injectionPoint_iter(package, id, iterId, fname,
                                               line);
}

void VnV::CppInjection::RegisterInjectionPoint(const char* package,
                                               const char* id,
                                               std::string json) {
  VnV::InjectionPointStore::instance().registerInjectionPoint(package, id,
                                                              json);
}
