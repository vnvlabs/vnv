#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "c-interfaces/PackageName.h"

//Define an interface for the OutputEngineWrapper
struct OutputEngineWrapper {
    void* ptr;
};

#define OUTPUTENGINESUPPORTEDTYPES X(double) X(int) X(float) X(long)
#define X(type) VNVEXTERNC void VnV_Output_Put_##type(struct OutputEngineWrapper *wrapper, const char* name, type* value);
OUTPUTENGINESUPPORTEDTYPES
#undef X
VNVEXTERNC void VnV_Output_Put_String(struct OutputEngineWrapper *wrapper, const char *name, const char* value);

struct ParameterDTO {
    const char * type;
    const char * rtti;
    const void * ptr;
};

//Define a wrapper and interface for the ParmaeterWrapper
struct ParameterSetWrapper {
   void* ptr;
};

VNVEXTERNC struct ParameterDTO VnV_Parameter_Get(struct ParameterSetWrapper *wrapper, const char* name);

//Define a callback that can be used to write injection point data
typedef void (*injectionDataCallback)(struct ParameterSetWrapper *wrapper, struct OutputEngineWrapper *engine );

#endif // WRAPPERS_H
