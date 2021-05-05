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



// Put comma before the first variable -- Because we have nothing after
#  define DOIT(X) , #  X, X
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)
namespace VnV {

enum class InjectionPointType;

namespace CppInjection {

typedef std::map<std::string, std::pair<std::string, void*>> NTV;
typedef std::function<void(
    VnV_Comm comm, std::map<std::string, VnVParameter>& ntv,
    OutputEngineManager* engine, InjectionPointType type, std::string stageId)>
    DataCallback;

void defaultCallBack(VnV_Comm comm, std::map<std::string, VnVParameter>& ntv,
                     IOutputEngine* engine, InjectionPointType type,
                     std::string stageId);


#  define VNV_CALLBACK                                                \
    [](VnV_Comm comm, std::map<std::string, VnVParameter> & ntv, \
              IOutputEngine * engine, InjectionPointType type,          \
              std::string stageId)

void UnwrapParameterPack(NTV& m);

void BeginPoint(VnV_Comm comm, const char* package, const char* id,
                const DataCallback& callback, NTV& map);

void BeginLoop(VnV_Comm comm, const char* package, const char* id,
               const DataCallback& callback, NTV& map);


VnV_Iterator BeginIteration(VnV_Comm comm, const char* package, const char* id,
                            const CppInjection::DataCallback& callback,
                            int once, NTV&inputs, NTV& ouputs);


void UnwrapParameterPack_iter(int inputs, NTV &mm, NTV& m);


int Iterate(VnV_Iterator* iterator);

bool EndLoop(const char* package, const char* id);
void IterLoop(const char* package, const char* id, const char* iterId);
void Register(const char* package, const char* id, bool iterative,  std::string parameters_str);

template <typename T, typename V, typename... Args>
void UnwrapParameterPack(NTV& m, V& name, T& first, Args&&... args) {
  m.insert(std::make_pair(
      name,
      std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  UnwrapParameterPack(m, std::forward<Args>(args)...);
}


template <typename T, typename V, typename... Args>
void UnwrapParameterPack_iter(int inputs, NTV& minputs, NTV&moutputs, V& name, T& first, Args&&... args) {
  if (minputs.size() < inputs ) {
    minputs.insert(std::make_pair(
      name,
      std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  } else {
      moutputs.insert(std::make_pair(
        name,
        std::make_pair(typeid(&first).name(), reinterpret_cast<void*>(&first))));
  }
  UnwrapParameterPack_iter(inputs, minputs, moutputs, std::forward<Args>(args)...);
}




template <typename A, typename... Args>
void BeginLoopPack(A comm, const char* package, const char* id,
                   const DataCallback& callback, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginLoop(comm, package, id, callback, m);
}

template <typename A, typename... Args>
void BeginPack(A comm, const char* package, const char* id,
               const DataCallback& callback, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> m;
  UnwrapParameterPack(m, std::forward<Args>(args)...);
  BeginPoint(comm, package, id, callback, m);
}

template <typename A, typename... Args>
VnV_Iterator IterationPack(A comm, const char* package, const char* id,
               const CppInjection::DataCallback& callback,
               int once, int inputs, Args&&... args) {
  std::map<std::string, std::pair<std::string, void*>> minputs;
  std::map<std::string, std::pair<std::string, void*>> moutputs;
  UnwrapParameterPack_iter(inputs, minputs, moutputs, std::forward<Args>(args)...);
  return BeginIteration(comm, package, id, callback, once, minputs, moutputs);
}



}  // namespace CppInjection
}  // namespace VnV

#  define INJECTION_POINT_C(PNAME, COMM, NAME, callback, ...) \
    VnV::CppInjection::BeginPack(createComm(COMM,PNAME), PNAME, NAME, \
                                 callback EVERYONE(__VA_ARGS__))

// SINGULAR INJECTION POINT.
#  define INJECTION_POINT(PNAME, COMM, NAME, ...)                             \
    INJECTION_POINT_C(PNAME, COMM, NAME, &VnV::CppInjection::defaultCallBack, \
                      __VA_ARGS__)

// BEGIN A LOOPED INJECTION POINT
#  define INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, ...) \
    VnV::CppInjection::BeginLoopPack(createComm(COMM, PNAME), PNAME, NAME,  \
                                     callback EVERYONE(__VA_ARGS__))

#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, ...) \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME,          \
                           &VnV::CppInjection::defaultCallBack, __VA_ARGS__)

// END A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_END(PNAME, NAME) \
    VnV::CppInjection::EndLoop(PNAME, NAME)

// INTERNAL ITERATION OF A LOOPED INJECTION POINT.
#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE) \
    VnV::CppInjection::IterLoop(PNAME, NAME, STAGE)


// Macro for an iterative vnv injection point.
# define INJECTION_ITERATION_C(VAR, PNAME, COMM, NAME, ONCE, INPUTS, callback, ...)\
   VnV_Iterator VAR = VnV::CppInjection::IterationPack(createComm(COMM, PNAME), PNAME, NAME, \
                    callback, ONCE, INPUTS EVERYONE(__VA_ARGS__));                                                                            \
   while(VnV::CppInjection::Iterate(&VAR))



#  define INJECTION_ITERATION(VAR, PNAME, COMM, NAME, ONCE, INPUTS, ...) \
    INJECTION_ITERATION_C(VAR, PNAME, COMM, NAME, ONCE, INPUTS,          \
                           &VnV::CppInjection::defaultCallBack, __VA_ARGS__)



VNVEXTERNC int  _VnV_injectionIterate(VnV_Iterator *iterator);

#  define INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function, callback, \
                                       ...)                                   \
    INJECTION_LOOP_BEGIN_C(PNAME, COMM, NAME, callback, __VA_ARGS__);         \
    function(__VA_ARGS__);                                                    \
    INJECTION_LOOP_END(PNAME, NAME)

#  define INJECTION_FUNCTION_WRAPPER(PNAME, COMM, NAME, function, ...) \
    INJECTION_FUNCTION_WRAPPER_C(PNAME, COMM, NAME, function,          \
                                 &VnV::CppInjection::defaultCallBack,  \
                                 __VA_ARGS__);

// REGISTER AN INJECTION POINT

#  define Register_Injection_Point(PNAME, NAME, ITERATIVE, PARAMETERS) \
    VnV::CppInjection::Register(PNAME, NAME, ITERATIVE, PARAMETERS);






#else

#  define INJECTION_POINT(...)
#  define INJECTION_LOOP_BEGIN(...)
#  define INJECTION_LOOP_END(...)
#  define INJECTION_LOOP_ITER(...)
#  define Register_Injection_Point(...)

#endif
#endif  // CPPINJECTION_H
