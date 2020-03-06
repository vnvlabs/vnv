#include "c-interfaces/CppInjection.h"
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"

void VnV::UnwrapParameterPack(NTV &/*m*/) {

}

void VnV::CppInjectionPoint_Begin(const char* package, const char* id, NTV &map) {
    VnV::RunTime::instance().injectionPoint_begin(package,id,map);
}

void VnV::CppInjectionPoint(const char* package, const char* id, NTV &map) {
    VnV::RunTime::instance().injectionPoint(package,id,map);
}

void VnV::CppInjectionPoint_End(const char* package, const char* id) {
    VnV::RunTime::instance().injectionPoint_end(package,id);
}
void VnV::CppInjectionPoint_Iter(const char* package, const char* id, const char* iterId) {
    VnV::RunTime::instance().injectionPoint_iter(package,id,iterId);
}

void VnV::CppRegisterInjectionPoint(std::string json){
    VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(json);
}
