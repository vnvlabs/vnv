#ifndef CPPINJECTIONPOINTINTERFACE_H
#define CPPINJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "json-schema.hpp"

typedef void (*vnv_registration_function)();

#  define IPCALLBACK                                                                                               \
    [](VnV_Comm comm, VnV::VnVParameterSet & ntv, VnV::OutputEngineManager * engine, VnV::InjectionPointType type, \
       std::string stageId)

// Put comma before the first variable -- Because we have nothing after
#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {

namespace CppInjection {

void UnwrapParameterPack(NTV& m);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(NTV& m, V& name, T& first, Args&&... args) {
  m.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  UnwrapParameterPack(m, std::forward<Args>(args)...);
}

void BeginPoint(VnV_Comm comm, const char* package, const char* id,  const VnV::TemplateCallback &templateCallback,
                const char* fname, int line, const DataCallback& callback, NTV& map);

void BeginLoop(VnV_Comm comm, const char* package, const char* id,  const VnV::TemplateCallback &templateCallback,
               const char* fname, int line, const DataCallback& callback, NTV& map);

void IterLoop(const char* package, const char* id, const char* iterId, const char* fname, int line);

bool EndLoop(const char* package, const char* id, const char* fname, int line);

void RegisterInjectionPoint(const char* package, const char* id, std::string json);

template <typename A, typename... Args>
void BeginLoopPack(A comm, const char* package, const char* id, const VnV::TemplateCallback &templateCallback, const char* fname, int line,
                   const DataCallback& callback, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginLoop(comm, package, id, templateCallback, fname, line, callback, m);
}

template <typename A, typename... Args>
void BeginPack(A comm, const char* package, const char* id, const VnV::TemplateCallback &templateCallback, const char* fname, int line, const DataCallback& callback,
               Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginPoint(comm, package, id, templateCallback, fname, line, callback, m);
}

}  // namespace CppInjection
}  // namespace VnV

  #define ITM(...) \
    VnV::TemplateCallback::create<__VA_ARGS__>(#__VA_ARGS__)
    

// BEGIN A SINGLE INJECTION POINT THAT HAS TEMPLATE PARAMETERS AND A CALLBACK. 
#  define INJECTION_POINT_TC(PNAME, COMM, NAME, templateCallback, callback, ...)          \
    VnV::CppInjection::BeginPack(COMM, PNAME, NAME, (templateCallback), __FILE__, __LINE__, \
                                 callback EVERYONE(__VA_ARGS__));

// BEGIN A LOOPED INJECTION POINT WITH TEMPLATES AND A CALLBACK
#  define INJECTION_LOOP_BEGIN_TC(PNAME, COMM, NAME, templateCallback, callback, ...)         \
    VnV::CppInjection::BeginLoopPack(COMM, PNAME, NAME, (templateCallback), __FILE__, __LINE__, \
                                     callback EVERYONE(__VA_ARGS__))

// SINGULAR INJECTION POINT NO CALLBACK BUT TEMPLATES
#  define INJECTION_POINT_T(PNAME, COMM, NAME, templateCallback, ...) \
    INJECTION_POINT_TC(PNAME, COMM, NAME, (templateCallback), &VnV::defaultCallBack, __VA_ARGS__)

#  define INJECTION_LOOP_BEGIN_T(PNAME, COMM, NAME, templateCallback, ...) \
    INJECTION_LOOP_BEGIN_TC(PNAME, COMM, NAME, (templateCallback), &VnV::defaultCallBack, __VA_ARGS__)

// INJECTION POINT WITH A CALLBACK BUT NO TEMPLATES
#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...) \
    INJECTION_POINT_TC(PNAME, COMM, NAME, VnV::TemplateCallback(), callback, __VA_ARGS__)

// BEGIN A LOOPED INJECTION POINT WITH A CALLBACK AND NO TEMPLATES
#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...) \
    INJECTION_LOOP_BEGIN_TC(PNAME, COMM, NAME, VnV::TemplateCallback(), callback, __VA_ARGS__)

// SINGULAR INJECTION POINT NO TEMPLATES AND NO CALLBACK.
#  define INJECTION_POINT(PNAME, COMM, NAME, ...) \
    INJECTION_POINT_C(PNAME, COMM, NAME, &VnV::defaultCallBack, __VA_ARGS__)

// INJECTION LOOP NO TEMPLATES AND NO CALLBACK
#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, &VnV::defaultCallBack, __VA_ARGS__)

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) VnV::CppInjection::IterLoop(PNAME, NAME, STAGE, __FILE__, __LINE__)

// END A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_END(PNAME, NAME) VnV::CppInjection::EndLoop(PNAME, NAME, __FILE__, __LINE__)

#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    VnV::CppInjection::RegisterInjectionPoint(PNAME, NAME, PARAMETERS);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_POINT_C(...)
#  define INJECTION_LOOP_BEGIN_C(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define INJECTION_FUNCTION_WRAPPER_C(...)
#  define INJECTION_FUNCTION_WRAPPER(...)
#  define Register_Injection_Point(...)

#endif
#endif  // CPPINJECTION_H
