#include "interfaces/CppInjection.h"
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"

void VnV::CppInjection::UnwrapParameterPack(NTV &/*m*/) {

}


void VnV::CppInjection::defaultCallBack(std::map<std::string,VnVParameter> &/*params*/, IOutputEngine* /*engine*/) {

}

void VnV::CppInjection::BeginLoop(const char* package, const char* id, const CppInjection::DataCallback &callback, NTV &map) {
    VnV::RunTime::instance().injectionPoint_begin(package,id,callback, map);
}

void VnV::CppInjection::BeginPoint(const char* package, const char* id, const CppInjection::DataCallback &callback, NTV &map) {
    VnV::RunTime::instance().injectionPoint(package,id,callback, map);
}

bool VnV::CppInjection::EndLoop(const char* package, const char* id) {
    VnV::RunTime::instance().injectionPoint_end(package,id);
    return true;
}
void VnV::CppInjection::IterLoop(const char* package, const char* id, const char* iterId) {
    VnV::RunTime::instance().injectionPoint_iter(package,id,iterId);
}

void VnV::CppInjection::Register(std::string json){
    VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(json);
}
