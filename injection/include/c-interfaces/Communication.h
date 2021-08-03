#ifndef CINTERFACE_COMMUNICATION_H
#define CINTERFACE_COMMUNICATION_H

#include "c-interfaces/PackageName.h"

#define MAX_PACKAGE_NAME_SIZE 20

struct VnV_Comm_ {
  const char *name;
  void* data;
};
typedef struct VnV_Comm_ VnV_Comm;

VNVEXTERNC VnV_Comm VnV_Comm_Self();
VNVEXTERNC VnV_Comm VnV_Comm_World();
VNVEXTERNC VnV_Comm VnV_Comm_Custom(const char* name, void* data);

#define VSELF VnV_Comm_Self()
#define VWORLD VnV_Comm_World()
#define VCUST(name, data) VnV_Comm_Custom(name, (void*) &data)
#define VMPI(data) VCUST("mpi",data)

#define INJECTION_EXECUTABLE(package,...) \
  INJECTION_REGISTRATION(package);

#endif  // COMMUNICATION_H
