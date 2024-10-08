﻿#ifndef CPPINJECTIONPOINTINTERFACE_H
#define CPPINJECTIONPOINTINTERFACE_H


#  include <iostream>
#  include <map>
#  include <string>

#  include "common-interfaces/all.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "validate/json-schema.hpp"

typedef void (*vnv_registration_function)();

#  define VNV_CALLBACK [&](VnV::VnVCallbackData & data)
#  define VNV_NOCALLBACK [](VnV::VnVCallbackData&) {}

// Put comma before the first variable -- Because we have nothing after
#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {
  
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

namespace CppInjection {

void UnwrapParameterPack(NTV& m);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(NTV& m, V& name, T& first, Args&&... args) {
  auto t = reinterpret_cast<void*>(&(const_cast<typename std::remove_const<T>::type&>(first)));
  m.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), t)));
  UnwrapParameterPack(m, std::forward<Args>(args)...);
}

void BeginLoop(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty, const char* fname,
               int line, const DataCallback& callback, NTV& map);

void IterLoop(const char* package, const char* id, std::string iterId, const char* fname, int line,
              const DataCallback& callback);

bool EndLoop(const char* package, const char* id, const char* fname, int line, const DataCallback& callback);

void RegisterInjectionPoint(const char* package, const char* id, std::string json);

template <typename A, typename... Args>
void BeginLoopPack(A comm, const char* package, const char* id, struct VnV_Function_Sig pretty, const char* fname,
                   int line, const DataCallback& callback, Args&&... args) {
  try {
    std::map<std::string, std::pair<std::string, void*>> m;
    UnwrapParameterPack(m, std::forward<Args>(args)...);
    BeginLoop(comm, package, id, pretty, fname, line, callback, m);
  } catch (std::exception& e) {
    VnV_Error(VNVPACKAGENAME, "Error Packing Injection Point: %s", e.what());
  }
}

}  // namespace CppInjection
}  // namespace VnV

#ifndef VNV_OFF

// BEGIN A LOOPED INJECTION POINT WITH TEMPLATES AND A CALLBACK
#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, callback, ...)                                                \
    VnV::CppInjection::BeginLoopPack(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, \
                                     callback EVERYONE(__VA_ARGS__))

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE, callback) \
    VnV::CppInjection::IterLoop(VNV_STR(PNAME), VNV_STR(NAME), STAGE, __FILE__, __LINE__, callback)

// END A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_END(PNAME, NAME, callback) \
    VnV::CppInjection::EndLoop(VNV_STR(PNAME), VNV_STR(NAME), __FILE__, __LINE__, callback)


// Injection point that begins and ends on the same line,
#  define INJECTION_POINT(PNAME, COMM, NAME, callback, ...)         \
    INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, callback, __VA_ARGS__); \
    INJECTION_LOOP_END(PNAME, NAME, VNV_NOCALLBACK);

#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) \
    VnV::CppInjection::RegisterInjectionPoint(PNAME, NAME, PARAMETERS);

#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif
#endif  // CPPINJECTION_H
