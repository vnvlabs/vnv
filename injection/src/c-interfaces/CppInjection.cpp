#include "c-interfaces/CppInjection.h"
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"

void VnV::UnwrapParameterPack(NTV &/*m*/) {

}

void VnV::CppInjectionPoint_Begin(std::string package, std::string id, NTV &map) {
    VnV::RunTime::instance().injectionPoint_begin(package,id,map);
}

void VnV::CppInjectionPoint(std::string package, std::string id, NTV &map) {
    VnV::RunTime::instance().injectionPoint(package,id,map);
}

void VnV::CppInjectionPoint_End(std::string package, std::string id) {
    VnV::RunTime::instance().injectionPoint_end(package,id);
}
void VnV::CppInjectionPoint_Iter(std::string package, std::string id, std::string iterId) {
    VnV::RunTime::instance().injectionPoint_iter(package,id,iterId);
}
void VnV::CppRegisterInjectionPoint(std::string json_str) {
    VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(json_str);
}
