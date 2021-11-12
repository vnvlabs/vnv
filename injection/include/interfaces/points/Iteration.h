#ifndef ITERATIONPOINTINTERFACE_H
#define ITERATIONPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "json-schema.hpp"

#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {

namespace CppIteration {

void UnwrapParameterPack(int inputs, NTV& mm, NTV& m);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(int inputs, NTV& minputs, NTV& moutputs, V& name, T& first, Args&&... args) {
  if (minputs.size() < inputs) {
    minputs.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  } else {
    moutputs.insert(std::make_pair(name, std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  }
  UnwrapParameterPack(inputs, minputs, moutputs, std::forward<Args>(args)...);
}

void Register(const char* package, const char* id, std::string json);

VnV_Iterator BeginIteration(VnV_Comm comm, const char* package, const char* id,
                            struct VnV_Function_Sig pretty, const char* fname, int line,
                            const DataCallback& callback, int once, NTV& inputs, NTV& ouputs);

int Iterate(VnV_Iterator* iterator);

template <typename A, typename... Args>
VnV_Iterator IterationPack(A comm, const char* package, const char* id, struct VnV_Function_Sig pretty,
                           const char* fname, int line, const DataCallback& callback, int once, int inputs,
                           Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> minputs;
  std::map<std::string, std::pair<std::string, void*>> moutputs;

  UnwrapParameterPack(inputs, minputs, moutputs, std::forward<Args>(args)...);

  return BeginIteration(comm, package, id, pretty, fname, line, callback, once, minputs, moutputs);
}

}  // namespace CppIteration
}  // namespace VnV

#  define INJECTION_ITERATION_C(VAR, PNAME, COMM, NAME, ONCE, INPUTS, callback, ...)          \
    VnV_Iterator VAR = VnV::CppIteration::IterationPack(COMM, PNAME, NAME, VNV_FUNCTION_SIG, __FILE__, __LINE__, \
                                                        callback, ONCE, INPUTS EVERYONE(__VA_ARGS__));           \
    while (VnV::CppIteration::Iterate(&VAR))

#  define INJECTION_ITERATION(VAR, PNAME, COMM, NAME, ONCE, INPUTS, ...) \
    INJECTION_ITERATION_C(VAR, PNAME, COMM, NAME, ONCE, INPUTS, &VnV::defaultCallBack, __VA_ARGS__)

#  define Register_Injection_Iterator(PNAME, NAME, PARAMETERS) VnV::CppIteration::Register(PNAME, NAME, PARAMETERS);

#else

#  define INJECTION_ITERATION_C(...)
#  define INJECTION_ITERATION(...)
#  define Register_Injection_Iterator(...)

#endif
#endif  // CPPINJECTION_H
