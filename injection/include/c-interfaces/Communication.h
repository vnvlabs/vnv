#ifndef CINTERFACE_COMMUNICATION_H
#define CINTERFACE_COMMUNICATION_H

#include "c-interfaces/PackageName.h"

#define MAX_PACKAGE_NAME_SIZE 45

struct VnV_Comm_ {
  char name[MAX_PACKAGE_NAME_SIZE + 1];
  void* data;
};
typedef struct VnV_Comm_ VnV_Comm;

VNVEXTERNC VnV_Comm VnV_Create_Comm(const char* package, void* data);
VNVEXTERNC VnV_Comm VnV_Comm_Self(const char* packageName);
VNVEXTERNC VnV_Comm VnV_Comm_World(const char* packageName);
VNVEXTERNC VnV_Comm VnV_Comm_Custom(const char* packageName, void* data);

VNVEXTERNC void VnV_Declare_Communicator(const char* packageName,
                                         const char* commPackage,
                                         const char* commName);



#ifdef __cplusplus

#include <cstring>

#define VSELF "self"
#define VWORLD "world"

VnV_Comm createComm(const char* str, const char* package);

template<typename T>
VnV_Comm createComm(T a, const char* package) {
  return VnV_Comm_Custom(package, (void*)&a);
}

#else

#define VSELF(PNAME) VnV_Comm_Self(VNV_STR(PNAME))
#define VWORLD(PNAME) VnV_Comm_World(VNV_STR(PNAME))
#define VCUST(PNAME,data) VnV_Comm_Custom(VNV_STR(PNAME), (void*) &data)

#endif

// This is used to define the communicator for the package that calls it.
// The Clang tool picks up this macro when defined and creates a registration
// call to define the communicator inside the registration call . If a call to
// this is not made, the default serial communicator is used.
#define INJECTION_COMMUNICATOR(package, commpackage, name)

#define INJECTION_EXECUTABLE(package, commpackage, name) \
  INJECTION_REGISTRATION(package);

#endif  // COMMUNICATION_H
