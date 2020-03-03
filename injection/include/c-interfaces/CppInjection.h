#ifndef CPPINJECTIONPOINTINTERFACE_H
#define CPPINJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#include "c-interfaces/PackageName.h"
#include <map>
#include <string>
#include "json-schema.hpp"

// Put comma before the first variable -- Because we have nothing after
#define DOIT(X)  ,#X, X
#define EVERYONE(...) FOR_EACH(DOIT,__VA_ARGS__)
#include <iostream>
namespace VnV {

typedef std::map<std::string, std::pair<std::string,void*>> NTV;

void UnwrapParameterPack(NTV &m);
void CppInjectionPoint(std::string package, std::string id, NTV &map);
void CppInjectionPoint_Begin(std::string package, std::string id, NTV &map);
void CppInjectionPoint_End(std::string package, std::string id);
void CppInjectionPoint_Iter(std::string package, std::string id, std::string iterId);
void CppRegisterInjectionPoint(std::string json_str);
void CppRegisterInjectionPoint(nlohmann::json &json);

template<typename T, typename V,  typename ...Args>
void UnwrapParameterPack(NTV &m, V& name, T& first, Args&&...args) {
    std::cout << name << "   " << typeid(first).name() << std::endl;

    m.insert(std::make_pair(name, std::make_pair(typeid(first).name(), static_cast<void*>(&first))));

    UnwrapParameterPack(m,std::forward<Args>(args)...);
}

template<typename ...Args>
void CppInjectionPoint_ParameterPack(std::string package, std::string id, bool loop, Args&&...args) {
    std::map<std::string,std::pair<std::string,void*>> m;
    UnwrapParameterPack(m,std::forward<Args>(args)...);
    if (loop) {
        CppInjectionPoint_Begin(package,id,m);
    } else {
        CppInjectionPoint(package,id,m);
    }
}
}

// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
    VnV::CppInjectionPoint_ParameterPack(VNV_STR(PACKAGENAME),#NAME,false  EVERYONE(__VA_ARGS__));

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VnV::CppInjectionPoint_ParameterPack(VNV_STR(PACKAGENAME), #NAME,true  EVERYONE(__VA_ARGS__));

// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME) \
    VnV::CppInjectionPoint_End(VNV_STR(PACKAGENAME), #NAME);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE) \
    VnV::CppInjectionPoint_Iter(VNV_STR(PACKAGENAME),#NAME,#STAGE);

//REGISTER AN INJECTION POINT
#define Register_Injection_Point(CONFIG) \
    VnV::CppRegisterInjectionPoint(CONFIG);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif

#endif // CPPINJECTION_H

