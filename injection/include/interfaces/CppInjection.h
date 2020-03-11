#ifndef CPPINJECTIONPOINTINTERFACE_H
#define CPPINJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#include <iostream>
#include "c-interfaces/PackageName.h"
#include <map>
#include <string>
#include "json-schema.hpp"
#include "interfaces/ITest.h"
#include "interfaces/IOutputEngine.h"

// Put comma before the first variable -- Because we have nothing after
#define DOIT(X)  ,#X, X
#define EVERYONE(...) FOR_EACH(DOIT,__VA_ARGS__)
namespace VnV {


namespace CppInjection {

typedef std::map<std::string, std::pair<std::string,void*>> NTV;
typedef std::function<void(std::map<std::string,VnVParameter> &ntv, IOutputEngine *engine)> DataCallback;

void defaultCallBack(std::map<std::string,VnVParameter> &ntv, IOutputEngine* engine);

void UnwrapParameterPack(NTV &m);
void BeginPoint(const char* package, const char * id, const DataCallback& callback, NTV &map);
void BeginLoop(const char *package, const char * id, const DataCallback&callback , NTV &map);
bool EndLoop(const char* package, const char* id);
void IterLoop(const char* package, const char * id, const char* iterId);
void Register(std::string json_str);

template<typename T, typename V,  typename ...Args>
void UnwrapParameterPack(NTV &m, V& name, T& first, Args&&...args) {
    m.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), static_cast<void*>(&first))));
    UnwrapParameterPack(m,std::forward<Args>(args)...);
}

template<typename ...Args>
void Begin(const char* package, const char * id, bool loop, const DataCallback &callback, Args&&...args) {
    std::map<std::string,std::pair<std::string,void*>> m;
    UnwrapParameterPack(m,std::forward<Args>(args)...);
    if (loop) {
        BeginLoop(package,id, callback,m);
    } else {
        BeginPoint(package,id,callback, m);
    }
}

}
}

// SINGULAR INJECTION POINT.
#define INJECTION_POINT(NAME, ...)        \
    VnV::CppInjection::Begin(VNV_STR(PACKAGENAME),#NAME,false, &VnV::CppInjection::defaultCallBack EVERYONE(__VA_ARGS__));

#define INJECTION_POINT_C(NAME, callback, ...)        \
    VnV::CppInjection::Begin(VNV_STR(PACKAGENAME),#NAME, false, callback EVERYONE(__VA_ARGS__));

// BEGIN A LOOPED INJECTION POINT
#define INJECTION_LOOP_BEGIN_C(NAME, callback, ...)        \
    VnV::CppInjection::Begin(VNV_STR(PACKAGENAME), #NAME,true, callback EVERYONE(__VA_ARGS__));

#define INJECTION_LOOP_BEGIN(NAME, ...)        \
    VnV::CppInjection::Begin(VNV_STR(PACKAGENAME), #NAME, true, &VnV::CppInjection::defaultCallBack EVERYONE(__VA_ARGS__));


// END A LOOPED INJECTION POINT.
#define INJECTION_LOOP_END(NAME) \
    VnV::CppInjection::EndLoop(VNV_STR(PACKAGENAME), #NAME);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#define INJECTION_LOOP_ITER(NAME,STAGE) \
    VnV::CppInjection::IterLoop(VNV_STR(PACKAGENAME),#NAME,#STAGE);

#define INJECTION_FUNCTION_WRAPPER(NAME, function,...) \
   INJECTION_LOOP_BEGIN(NAME,__VA_ARGS__) \
   function(__VA_ARGS__); \
   INJECTION_LOOP_END(NAME,__VA_ARGS__);


//REGISTER AN INJECTION POINT
#define Register_Injection_Point(CONFIG) \
    VnV::CppInjection::Register(CONFIG);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif

#endif // CPPINJECTION_H

