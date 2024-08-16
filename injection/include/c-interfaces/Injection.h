#ifndef INJECTIONPOINTINTERFACE_H
#define INJECTIONPOINTINTERFACE_H

// if not NOT using VnV
//  i.e., if using VnV

#  include "c-interfaces/Wrappers.h"
#  include "common-interfaces/all.h"

VNVEXTERNC void _VnV_injectionPoint_begin(VnV_Comm comm, const char* packageName, const char* id,
                                          struct VnV_Function_Sig pretty, const char* fname, int line,
                                          injectionDataCallback callback, ...);

VNVEXTERNC void _VnV_injectionPoint_loop(const char* packageName, const char* id, const char* stageId,
                                         const char* fname, int line, injectionDataCallback callback);

VNVEXTERNC void _VnV_injectionPoint_end(const char* packageName, const char* id, const char* fname, int line,
                                        injectionDataCallback callback);

VNVEXTERNC void _VnV_registerInjectionPoint(const char* packageName, const char* id, const char* parameters_str);


#  define DOIT(X) #  X, (void*)(&X),
#  define EVERYONE(...) FOR_EACH(DOIT, __VA_ARGS__)
#  define VNV_NOCALLBACK NULL

#  define INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, callback, ...)                                                   \
    _VnV_injectionPoint_begin(COMM, VNV_STR(PNAME), VNV_STR(NAME), VNV_FUNCTION_SIG, __FILE__, __LINE__, callback, \
                              EVERYONE(__VA_ARGS__) VNV_END_PARAMETERS_S);

#  define INJECTION_LOOP_END(PNAME, NAME, CALLBACK) \
    _VnV_injectionPoint_end(VNV_STR(PNAME), VNV_STR(NAME), __FILE__, __LINE__, CALLBACK);

#  define INJECTION_LOOP_ITER(PNAME, NAME, STAGE, CALLBACK) \
    _VnV_injectionPoint_loop(VNV_STR(PNAME), VNV_STR(NAME), STAGE, __FILE__, __LINE__, CALLBACK);

#  define INJECTION_POINT(PNAME, COMM, NAME, CALLBACK, ...)        \
    INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, CALLBACK, __VA_ARGS__) \
    INJECTION_LOOP_END(PNAME, NAME, NULL, VNV_NOCALLBACK)

#define INJECTION_EXT_CALLBACK(PNAME, NAME)\
  void *NAME##_CALLBACK__EXTD = NULL;\
  void NAME##_CALLBACK__EXT1(VnV_Comm comm, struct ParameterSetWrapper* wrapper, struct IOutputEngineWrapper* engine, int injectionPointType, const char* stageId, void* data);\
  void NAME##_CALLBACK__EXT(VnV_Comm comm, struct ParameterSetWrapper* wrapper, struct IOutputEngineWrapper* engine, int injectionPointType, const char* stageId) {\
    NAME##_CALLBACK__EXT1(comm, wrapper, engine, injectionPointType, stageId, NAME##_CALLBACK__EXTD);\
  }\
  injectionDataCallback NAME##_CALLBACK_EXT2(void* data) { \
    NAME##_CALLBACK__EXTD = data;  \
    return NAME##_CALLBACK__EXT;  \
  }  \
  void NAME##_CALLBACK__EXT1(VnV_Comm comm, struct ParameterSetWrapper* wrapper, struct IOutputEngineWrapper* engine, int injectionPointType, const char* stageId, void* data)
  
#define INJECTION_EXT_CALLBACK_FWD(PNAME, NAME)\
  injectionDataCallback NAME##_CALLBACK_EXT2(void* data);
  
#define INJECTION_LOOP_BEGIN_WITH_EXT_CALLBACK(PNAME, COMM, NAME, DATA, ...)\
      INJECTION_LOOP_BEGIN(PNAME, COMM, NAME, NAME##_CALLBACK_EXT2(DATA), __VA_ARGS__);

 #define INJECTION_LOOP_END_WITH_EXT_CALLBACK(PNAME, NAME, DATA)\
      INJECTION_LOOP_END(PNAME, NAME, NAME##_CALLBACK_EXT2(DATA));

 #define INJECTION_LOOP_ITER_WITH_EXT_CALLBACK(PNAME, NAME, STAGE, DATA)\
      INJECTION_LOOP_ITER(PNAME, NAME, STAGE, NAME##_CALLBACK_EXT2(DATA));

// REGISTER AN INJECTION POINT
#  define Register_Injection_Point(PNAME, NAME, PARAMETERS) _VnV_registerInjectionPoint(PNAME, NAME, PARAMETERS);



#endif  // INJECTIONPOINTINTERFACE_H
