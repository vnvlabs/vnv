﻿#ifndef PLUGPOINTINTERFACE_H
#define PLUGPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "common-interfaces/Communication.h"
#  include "common-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "interfaces/points/Iteration.h"
#  include "json-schema.hpp"

#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {

namespace CppPlug {

void Register(const char* package, const char* id, std::string json);

VnV_Iterator BeginPlug(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                       const char* fname, int line, const DataCallback& callback, NTV& parameters);

int Iterate(VnV_Iterator* iterator);

template <typename A, typename... Args>
VnV_Iterator PlugPack(A comm, const char* package, const char* id, struct VnV_Function_Sig pretty, const char* fname,
                      int line, const DataCallback& callback, Args&&... args) {
  try {
    std::map<std::string, std::pair<std::string, void*>> parameters;
    CppIteration::UnwrapParameterPack(parameters, std::forward<Args>(args)...);

    return BeginPlug(comm, package, id, pretty, fname, line, callback, parameters);

  } catch (std::exception& e) {
    assert(false && "cant happen as we cant handle once parameter from here");
    return {NULL};
  }
}

}  // namespace CppPlug
}  // namespace VnV

// Macro for an iterative vnv injection point.
#  define INJECTION_FUNCTION_PLUG(VAR, PNAME, COMM, NAME, callback, ...)                                       \
    VnV_Iterator VAR = VnV::CppPlug::PlugPack(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, \
                                              __LINE__, callback EVERYONE(__VA_ARGS__));                       \
    while (VnV::CppPlug::Iterate(&VAR))

#  define Register_Injection_Plug(PNAME, NAME, PARAMETERS) VnV::CppPlug::Register(PNAME, NAME, PARAMETERS);

#else

#  define INJECTION_FUNCTION_PLUG_C(...)
#  define INJECTION_FUNCTION_PLUG(...)
#  define Register_Injection_Plug(...)

#endif
#endif  // CPPINJECTION_H
