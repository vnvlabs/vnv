﻿#include "c-interfaces/Communication.h"

#include <cstring>
#include <sstream>

#include "base/exceptions.h"
#include "base/stores/CommunicationStore.h"
#include "c-interfaces/Logging.h"

extern "C" {

VnV_Comm VnV_Comm_Self() { 
  try {
    return VnV::CommunicationStore::instance().self();
  } catch (...) {
    VnV_Error(VNVPACKAGENAME,"Error fetching communicator");  
    return {"Error",NULL};
  } }

VnV_Comm VnV_Comm_World() {
  try {
    return VnV::CommunicationStore::instance().world();
  } catch (...) {
    VnV_Error(VNVPACKAGENAME,"Error fetching communicator");  
    return {"Error",NULL};
  }
}

VnV_Comm VnV_Comm_Custom(const char* name, void* data) {
  try {
    return VnV::CommunicationStore::instance().custom(name,data);
  } catch (...) {
    VnV_Error(VNVPACKAGENAME,"Error fetching communicator");  
    return {"Error",NULL};
  }
}
}