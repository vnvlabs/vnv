#include "c-interfaces/Communication.h"
#include <cstring>
#include <sstream>

#include "base/CommunicationStore.h"
#include "base/exceptions.h"

extern "C" {

VnV_Comm VnV_Comm_Self(const char* packageName) {
  return VnV::CommunicationStore::instance().selfData(packageName);
}

VnV_Comm VnV_Comm_World(const char* packageName) {
  return VnV::CommunicationStore::instance().worldData(packageName);
}

VnV_Comm VnV_Comm_Custom(const char* packageName, void* data) {
  return VnV::CommunicationStore::instance().customData(packageName, data);
}

void VnV_Declare_Communicator(const char* packageName, const char* commPackage,
                              const char* commName) {
  VnV::CommunicationStore::instance().declareComm(packageName, commPackage,
                                                  commName);
}

VnV_Comm VnV_Create_Comm(const char* packageName, void* data) {
  auto length = std::strlen(packageName);
  if (length > MAX_PACKAGE_NAME_SIZE) {
    std::ostringstream oss;
    oss << "Package Name is to long: " << packageName << "(length = " << length
        << ")";
    throw VnV::VnVExceptionBase(oss.str());
  }
  VnV_Comm comm;
  std::strcpy(comm.name, packageName);
  comm.data = data;
  return comm;
}
}
