#ifndef ITERATIONPOINTINTERFACE_H
#define ITERATIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "common-interfaces/Communication.h"
#  include "common-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "json-schema.hpp"

#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {

namespace CppIteration {

void UnwrapParameterPack(NTV& m);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(NTV& m, V& name, T& first, Args&&... args) {
  m.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  UnwrapParameterPack(m, std::forward<Args>(args)...);
}

void Register(const char* package, const char* id, std::string json);

VnV_Iterator BeginIteration(VnV_Comm comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                            const char* fname, int line, const DataCallback& callback, int once, NTV& parameters);

int Iterate(VnV_Iterator* iterator);

template <typename A, typename... Args>
VnV_Iterator IterationPack(A comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                           const char* fname, int line, const DataCallback& callback, int once, Args&&... args) {
  try {
    std::map<std::string, std::pair<std::string, void*>> parameters;

    UnwrapParameterPack(parameters, std::forward<Args>(args)...);

    return BeginIteration(comm, package, id, pretty, fname, line, callback, once, parameters);

  } catch (std::exception& e) {
    assert(false && "cant happen as we cant handle once parameter from here");
    return {NULL};
  }
}

}  // namespace CppIteration
}  // namespace VnV

#  define INJECTION_ITERATION(VAR, PNAME, COMM, NAME, ONCE, callback, ...)                                         \
    VnV_Iterator VAR = VnV::CppIteration::IterationPack(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG,     \
                                                        __FILE__, __LINE__, callback, ONCE EVERYONE(__VA_ARGS__)); \
    while (VnV::CppIteration::Iterate(&VAR))

#  define Register_Injection_Iterator(PNAME, NAME, PARAMETERS) VnV::CppIteration::Register(PNAME, NAME, PARAMETERS);

#else

#  define INJECTION_ITERATION_C(...)
#  define INJECTION_ITERATION(...)
#  define Register_Injection_Iterator(...)

#endif
#endif  // CPPINJECTION_H
