#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"

//Define the inters for the VnV Iterator 
typedef struct VnV_Iterator_ {
   void* data;
} VnV_Iterator;


// Define an interface for the OutputEngineWrapper
struct IOutputEngineWrapper {
  void* ptr;
};

#define OUTPUTENGINESUPPORTEDTYPES X(double) X(int) X(long)
#define X(type)                                                               \
  VNVEXTERNC void VnV_Output_Put_##type(                        \
                                        struct IOutputEngineWrapper* wrapper, \
                                        const char* name, type* value);
OUTPUTENGINESUPPORTEDTYPES
#undef X

VNVEXTERNC void VnV_Output_Put_String(struct IOutputEngineWrapper* wrapper,
                                      const char* name, const char* value);

struct ParameterDTO {
  const char* type;
  const char* rtti;
  const void* ptr;
};

// Define a wrapper and interface for the ParmaeterWrapper
struct ParameterSetWrapper {
  void* ptr;
};

#define InjectionPointType_Single 0
#define InjectionPointType_Begin 1
#define InjectionPointType_Iter 2
#define InjectionPointType_End 3

VNVEXTERNC struct ParameterDTO VnV_Parameter_Get(
    struct ParameterSetWrapper* wrapper, const char* name);

// Define a callback that can be used to write injection point data
typedef void (*injectionDataCallback)(VnV_Comm comm,
                                      struct ParameterSetWrapper* wrapper,
                                      struct IOutputEngineWrapper* engine,
                                      int injectionPointType,
                                      const char* stageId);

#endif  // WRAPPERS_H
