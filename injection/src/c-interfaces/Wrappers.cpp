

#include "c-interfaces/Wrappers.h"
#include "base/InjectionPoint.h"
#include "interfaces/ITest.h"
#include <map>

namespace VnV {

namespace {
    VnVParameterSet* ParameterWrapperCast(ParameterSetWrapper *wrapper) {
      return static_cast<VnVParameterSet*>(wrapper->ptr);
    }

    IOutputEngine* EngineWrapperCast(IOutputEngineWrapper *wrapper) {
      return static_cast<IOutputEngine*>(wrapper->ptr);
    }

}
}
extern "C" {

// Do most of the Put Commands using X Macros.
#define X(type) \
    void VnV_Output_Put_##type(VnV_Comm comm, IOutputEngineWrapper *wrapper, const char* name, type* value) {\
    VnV::EngineWrapperCast(wrapper)->Put(comm, name,*value);\
}

OUTPUTENGINESUPPORTEDTYPES

#undef X

//An extra one to handle char*
void VnV_Output_Put_String(VnV_Comm comm, IOutputEngineWrapper *wrapper, const char *name, const char* value) {
    // TODO add "const" to the def on Put(std::string &string);
    std::string nameS = name;
    std::string valueS = value;
    VnV::EngineWrapperCast(wrapper)->Put(comm, nameS,valueS);
}

ParameterDTO VnV_Parameter_Get(ParameterSetWrapper *wrapper, const char* name) {
    VnV::VnVParameterSet *s = VnV::ParameterWrapperCast(wrapper);
    auto it = s->find(name);
    if ( it != s->end() ) {
        return {it->second.getType().c_str(), it->second.getRtti().c_str(),it->second.getRawPtr()};
    } else {
        return {nullptr,nullptr,nullptr};
    }
}

}

