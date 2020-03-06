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
void CppInjectionPoint(const char* package, const char * id, NTV &map);
void CppInjectionPoint_Begin(const char *package, const char * id, NTV &map);
void CppInjectionPoint_End(const char* package, const char* id);
void CppInjectionPoint_Iter(const char* package, const char * id, const char* iterId);
void CppRegisterInjectionPoint(std::string json_str);

template<typename T, typename V,  typename ...Args>
void UnwrapParameterPack(NTV &m, V& name, T& first, Args&&...args) {
    std::cout << name << "   " << typeid(first).name() << std::endl;

    m.insert(std::make_pair(name, std::make_pair(typeid(first).name(), static_cast<void*>(&first))));

    UnwrapParameterPack(m,std::forward<Args>(args)...);
}

template<typename ...Args>
void CppInjectionPoint_ParameterPack(const char* package, const char * id, bool loop, Args&&...args) {
    std::map<std::string,std::pair<std::string,void*>> m;
    UnwrapParameterPack(m,std::forward<Args>(args)...);
    if (loop) {
        CppInjectionPoint_Begin(package,id,m);
    } else {
        CppInjectionPoint(package,id,m);
    }
}

template<typename ...Args>
void _CppInjectionPoint(const char *package, const char* name, Args&&...args) {
    CppInjectionPoint_ParameterPack(package,name,false,std::forward<Args>(args)...);
}

template<typename ...Args>
void _CppInjectionPoint_Begin(const char * package, const char* name, Args&&...args) {
    CppInjectionPoint_ParameterPack(package,name,true,std::forward<Args>(args)...);
}

}

// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
    VnV::_CppInjectionPoint(VNV_STR(PACKAGENAME),#NAME EVERYONE(__VA_ARGS__));

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VnV::_CppInjectionPoint_Begin(VNV_STR(PACKAGENAME), #NAME EVERYONE(__VA_ARGS__));

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

