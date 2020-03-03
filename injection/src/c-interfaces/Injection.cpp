
/**
  @file Injection.cpp
**/
#include <stdarg.h>
#include "base/Runtime.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/PackageName.h"

typedef std::map<std::string,std::pair<std::string, void*>> NTV;

/** For another time -- We should implement a test parameter class.
class TestParameter  {
private:
    void *ptr;
    std::string type;
    std::string rtti;
public:
    TestParameter(std::string type, std::string rtti, void *ptr) {
        this->ptr = ptr;
        this->type = type;
        this->rtti = rtti;
    }

    template <typename T>
    TestParameter(std::string type, const T& object) :
        TestParameter(type, typeid(object).name(), rtti,static_cast<void*>(&object)){}

    bool isType(std::string type) {
        return (this->type == type);
    }

    std::string getType() {
        return type;
    }

    std::string getRTTI() {
        return rtti;
    }

    template <typename T>
    const T* get() {
        return static_cast<T*>(ptr);
    }
};*/


// Private function -- Never exposed to the C interface.
NTV VnV_UnwrapVariadicArgs(va_list argp) {
    NTV ntv;
    while (1) {
      std::string variableName = va_arg(argp, char*);
      if (variableName == VNV_END_PARAMETERS_S) {
        break;
      }
      void* variablePtr = va_arg(argp, void*);

      auto it = ntv.find(variableName);
      if (it!=ntv.end()) {
          it->second.second = variablePtr;
      } else {
          //variable was not registered, add it with a type void*
          ntv.insert(std::make_pair(variableName, std::make_pair("void*", variablePtr)));
      }
    }
    return ntv;
}

extern "C" {

void _VnV_injectionPoint(const char *package, const char* id, ...) {
  va_list argp;
  va_start(argp, id);
  NTV map = VnV_UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint(package, id, map);
  va_end(argp);
}

void _VnV_injectionPoint_begin(const char *package, const char* id, ...) {
  va_list argp;
  va_start(argp, id);
  NTV map = VnV_UnwrapVariadicArgs(argp);
  VnV::RunTime::instance().injectionPoint_begin(package, id, map);
  va_end(argp);
}

void _VnV_injectionPoint_end(const char * package, const char* id){
  VnV::RunTime::instance().injectionPoint_end(package, id);
}

void _VnV_injectionPoint_loop(const char * package, const char* id, const char* stageId){
  VnV::RunTime::instance().injectionPoint_iter(package, id, stageId);
}

void _VnV_registerInjectionPoint( const char *json_str) {
   VnV::InjectionPointStore::getInjectionPointStore().registerInjectionPoint(json_str);
}


}
