#include "interfaces/points/Plug.h"
#include "base/stores/PlugStore.h"
#include "base/Runtime.h"


void VnV::CppPlug::Register(const char* package, const char* id, std::string json) {
  VnV::PlugStore::getPlugStore().registerPlug(package, id, json);
}


VnV_Iterator VnV::CppPlug::BeginPlug(VnV_Comm comm, const char* package,
                                               const char* id, DataCallback callback, NTV&inputs, NTV &outputs) {
    return VnV::RunTime::instance().injectionPlug(comm, package,id, callback, inputs, outputs);
}

int VnV::CppPlug::Iterate(VnV_Iterator* iterator) {
    return VnV::RunTime::instance().injectionPlugRun(iterator);
}


