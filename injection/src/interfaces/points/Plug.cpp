#include "interfaces/points/Plug.h"

#include "base/Runtime.h"
#include "base/stores/PlugStore.h"

void VnV::CppPlug::Register(const char* package, const char* id, std::string json) {
  try {
    VnV::PlugStore::instance().registerPlug(package, id, json);

  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Registering Plug %s:%s", package, id);
  }
}

VnV_Iterator VnV::CppPlug::BeginPlug(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                                     const char* fname, int line, DataCallback callback, NTV& parameters) {
  try {
    return VnV::RunTime::instance().injectionPlug(comm, package, id, pretty, fname, line, callback, parameters);

  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running Loop %s:%s", package, id);
    return {NULL};
  }
}

int VnV::CppPlug::Iterate(VnV_Iterator* iterator) {
  try {
    return VnV::RunTime::instance().injectionPlugRun(iterator);

  } catch (...) {
    VnV_Error(VNVPACKAGENAME, "Error Running iterate");
    return 0;
  }
}
