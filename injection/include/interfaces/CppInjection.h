#ifndef CPPINJECTIONPOINTINTERFACE_H
#define CPPINJECTIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "c-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "json-schema.hpp"

typedef void (*vnv_registration_function)();

// Put comma before the first variable -- Because we have nothing after
#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)
namespace VnV {

namespace CppInjection {

typedef std::map<std::string, std::pair<std::string, void*>> NTV;
typedef std::function<void(VnV_Comm comm,
                           std::map<std::string, VnVParameter>& ntv,
                           OutputEngineManager* engine)>
    DataCallback;

void defaultCallBack(VnV_Comm comm, std::map<std::string, VnVParameter>& ntv,
                     IOutputEngine* engine);

void UnwrapParameterPack(NTV& m);
void BeginPoint(VnV_Comm comm, vnv_registration_function reg,
                const char* package, const char* id,
                const DataCallback& callback, NTV& map);
void BeginLoop(VnV_Comm comm, vnv_registration_function reg,
               const char* package, const char* id,
               const DataCallback& callback, NTV& map);
bool EndLoop(const char* package, const char* id);
void IterLoop(const char* package, const char* id, const char* iterId);
void Register(const char* package, const char* id, std::string parameters_str);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(NTV& m, V& name, T& first, Args&&... args) {
  m.insert(std::make_pair(
      name, std::make_pair(typeid(&first).name(), static_cast<void*>(&first))));
  UnwrapParameterPack(m, std::forward<Args>(args)...);
}

template <typename... Args>
void BeginLoopPack(VnV_Comm comm, const char* package, const char* id,
                   const DataCallback& callback, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginLoop(comm, INJECTION_REGISTRATION_PTR, package, id, callback, m);
}

template <typename... Args>
void BeginPack(VnV_Comm comm, const char* package, const char* id,
               const DataCallback& callback, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginPoint(comm, INJECTION_REGISTRATION_PTR, package, id, callback, m);
}

}  // namespace CppInjection
}  // namespace VnV

#  define INJECTION_POINT_C(COMM, NAME, callback, ...)              \
    VnV::CppInjection::BeginPack(COMM, VNV_STR(PACKAGENAME), #NAME, \
                                 callback EVERYONE(__VA_ARGS__));

// SINGULAR INJECTION POINT.
#  define INJECTION_POINT(COMM, NAME, ...)                             \
    INJECTION_POINT_C(COMM, NAME, &VnV::CppInjection::defaultCallBack, \
                      __VA_ARGS__);

// BEGIN A LOOPED INJECTION POINT
#  define INJECTION_LOOP_BEGIN_C(COMM, NAME, callback, ...)             \
    VnV::CppInjection::BeginLoopPack(COMM, VNV_STR(PACKAGENAME), #NAME, \
                                     callback EVERYONE(__VA_ARGS__));

#  define INJECTION_LOOP_BEGIN(COMM, NAME, ...)                             \
    INJECTION_LOOP_BEGIN_C(COMM, NAME, &VnV::CppInjection::defaultCallBack, \
                           __VA_ARGS__)

// END A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_END(NAME) \
    VnV::CppInjection::EndLoop(VNV_STR(PACKAGENAME), #NAME);

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_ITER(NAME, STAGE) \
    VnV::CppInjection::IterLoop(VNV_STR(PACKAGENAME), #NAME, #STAGE);

#  define INJECTION_FUNCTION_WRAPPER_C(COMM, NAME, function, callback, ...) \
    INJECTION_LOOP_BEGIN_C(COMM, NAME, function, __VA_ARGS__);              \
    function(__VA_ARGS__);                                                  \
    INJECTION_LOOP_END(COMM, NAME)

#  define INJECTION_FUNCTION_WRAPPER(COMM, NAME, function, ...)       \
    INJECTION_FUNCTION_WRAPPER_C(COMM, NAME, function,                \
                                 &VnV::CppInjection::defaultCallBack, \
                                 __VA_ARGS__);

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(NAME, PARAMETERS) \
    VnV::CppInjection::Register(PACKAGENAME_S, #NAME, PARAMETERS);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif
#endif  // CPPINJECTION_H
