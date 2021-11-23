#include "interfaces/points/Plug.h"

#include "base/Runtime.h"
#include "base/stores/PlugStore.h"

void VnV::CppPlug::Register(const char* package, const char* id,
                            std::string json) {
  VnV::PlugStore::instance().registerPlug(package, id, json);
}

VnV_Iterator VnV::CppPlug::BeginPlug(VnV_Comm comm, const char* package,
                                     const char* id, struct VnV_Function_Sig pretty,
                                     const char* fname,
                                     int line, DataCallback callback,
                                     NTV& inputs, NTV& outputs) {
  return VnV::RunTime::instance().injectionPlug(comm, package, id,pretty,  fname, line,
                                                callback, inputs, outputs);
}

int VnV::CppPlug::Iterate(VnV_Iterator* iterator) {
  return VnV::RunTime::instance().injectionPlugRun(iterator);
}
