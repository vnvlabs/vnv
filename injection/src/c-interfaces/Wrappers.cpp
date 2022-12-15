

#include "c-interfaces/Wrappers.h"

#include <map>

#include "base/points/InjectionPoint.h"
#include "interfaces/ITest.h"

namespace VnV {

namespace {
VnVParameterSet* ParameterWrapperCast(ParameterSetWrapper* wrapper) {
  return static_cast<VnVParameterSet*>(wrapper->ptr);
}

IOutputEngine* EngineWrapperCast(IOutputEngineWrapper* wrapper) {
  return static_cast<IOutputEngine*>(wrapper->ptr);
}



}  // namespace

  DataCallback DataCallback_wrapper(injectionDataCallback callback) {
     
     return [callback](VnVCallbackData& callback1){
        
        if (callback != NULL) { 
          IOutputEngineWrapper engineWraper = {static_cast<void*>(callback1.engine->getOutputEngine())};
          ParameterSetWrapper paramWrapper = {static_cast<void*>(&callback1.ntv)};
          int t = InjectionPointTypeUtils::toC(callback1.type);
          (callback)(callback1.comm, &paramWrapper, &engineWraper, t, callback1.stageId.c_str());
        }

     };

  }


}  // namespace VnV
extern "C" {

// Do most of the Put Commands using X Macros.
#define X(type)                                                               \
  void VnV_Output_Put_##type(IOutputEngineWrapper* wrapper, const char* name, \
                             type* value) {                                   \
    VnV::EngineWrapperCast(wrapper)->Put(name, *value);                       \
  }

OUTPUTENGINESUPPORTEDTYPES

#undef X

// An extra one to handle char*
void VnV_Output_Put_String(IOutputEngineWrapper* wrapper, const char* name,
                           const char* value) {
  std::string nameS = name;
  std::string valueS = value;
  VnV::EngineWrapperCast(wrapper)->Put(nameS, valueS);
}

ParameterDTO VnV_Parameter_Get(ParameterSetWrapper* wrapper, const char* name) {
  VnV::VnVParameterSet* s = VnV::ParameterWrapperCast(wrapper);
  auto it = s->find(name);
  if (it != s->end()) {
    return {it->second.getType().c_str(), it->second.getRawPtr()};
  } else {
    return {nullptr, nullptr};
  }
}
}

