#ifndef PLUGPOINTINTERFACE_H
#define PLUGPOINTINTERFACE_H

#ifndef WITHOUT_VNV

#  include <iostream>
#  include <map>
#  include <string>

#  include "c-interfaces/Communication.h"
#  include "c-interfaces/PackageName.h"
#  include "interfaces/IOutputEngine.h"
#  include "interfaces/ITest.h"
#  include "json-schema.hpp"

#  include "interfaces/points/Iteration.h"

#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)

namespace VnV {

namespace CppPlug {


void Register(const char* package, const char* id, std::string json);


VnV_Iterator BeginPlug(VnV_Comm comm, const char* package, const char* id, const char* fname, int line, DataCallback callback,  NTV&inputs, NTV& outputs);
int Iterate(VnV_Iterator* iterator);


template <typename A, typename... Args>
VnV_Iterator PlugPack(A comm, const char* package, const char* id, const char* fname, int line, 
               const DataCallback& callback,
               int inputs, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> minputs;
  std::map<std::string, std::pair<std::string, void*>> moutputs;
  CppIteration::UnwrapParameterPack(inputs, minputs, moutputs, std::forward<Args>(args)...);
  return BeginPlug(comm, package, id, fname, line, callback, minputs, moutputs);
}


}  // namespace CppInjection
}  // namespace VnV

// Macro for an iterative vnv injection point.
# define INJECTION_FUNCTION_PLUG_C(VAR, PNAME, COMM, NAME, INPUTS, callback, ...)\
   VnV_Iterator VAR = VnV::CppPlug::PlugPack(COMM, PNAME, NAME, __FILE__,__LINE__, callback, INPUTS EVERYONE(__VA_ARGS__));                                                                            \
   while(VnV::CppPlug::Iterate(&VAR))

#  define INJECTION_FUNCTION_PLUG(VAR, PNAME, COMM, NAME, INPUTS, ...) \
    INJECTION_FUNCTION_PLUG_C(VAR, PNAME, COMM, NAME, INPUTS, &VnV::defaultCallBack, __VA_ARGS__)

#  define Register_Injection_Plug(PNAME, NAME, PARAMETERS) \
    VnV::CppPlug::Register(PNAME, NAME, PARAMETERS);


#else

#  define INJECTION_FUNCTION_PLUG_C(...)
#  define INJECTION_FUNCTION_PLUG(...)
#  define Register_Injection_Plug(...)

#endif
#endif  // CPPINJECTION_H
