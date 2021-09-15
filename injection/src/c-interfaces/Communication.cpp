#include "c-interfaces/Communication.h"

#include <cstring>
#include <sstream>

#include "base/exceptions.h"
#include "base/stores/CommunicationStore.h"

extern "C" {

VnV_Comm VnV_Comm_Self() { return VnV::CommunicationStore::instance().self(); }

VnV_Comm VnV_Comm_World() {
  return VnV::CommunicationStore::instance().world();
}

VnV_Comm VnV_Comm_Custom(const char* name, void* data) {
  return VnV::CommunicationStore::instance().custom(name, data);
}
}