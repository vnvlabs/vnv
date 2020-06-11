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

#define VSELF \
  VnV_Comm_Self(PACKAGENAME_S)  // Built in comm for serial programs, (like
                                // MPI_COMM_SELF));
#define VWORLD VnV_Comm_World(PACKAGENAME_S)
#define VCUST(data) VnV_Comm_Custom(PACKAGENAME_S, data)
#define VCOMM \
  VnV_Comm_Custom(PACKAGENAME_S, comm)  // Assumes a communicator called comm.

// This is used to define the communicator for the package that calls it.
// The Clang tool picks up this macro when defined and creates a registration
// call to define the communicator inside the registration call . If a call to
// this is not made, the default serial communicator is used.
#define INJECTION_COMMUNICATOR(package, name)

#endif  // COMMUNICATION_H
