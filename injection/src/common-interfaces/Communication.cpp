
#include <cstring>
#include <sstream>

#include "common-interfaces/all.h"
#include "shared/exceptions.h"
#include "base/stores/CommunicationStore.h"
#include "common-interfaces/all.h"



extern "C" {

VnV_Comm VnV_Comm_Self() {
    return VnV::CommunicationStore::instance().self();
}

VnV_Comm VnV_Comm_World() {
    return VnV::CommunicationStore::instance().world();
}

#ifndef WITHOUT_MPI
VnV_Comm VnV_Comm_Cust(MPI_Comm comm) {
   return VnV::CommunicationStore::instance().custom(comm); 
}
#endif


}