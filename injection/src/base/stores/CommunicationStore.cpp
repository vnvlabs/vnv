
#include "base/stores/CommunicationStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"



using VnVCommType = VnV::VnVCommType;


namespace VnV {
namespace Communication {

VnV::ICommunicator* create_mpi_communicator(MPI_Comm comm);
VnV::ICommunicator* create_mpi_communicator(VnVCommType type);
}  // namespace VNVPACKAGENAME
}  // namespace VnV

namespace VnV {

constexpr const char* self_str = "s";
constexpr const char* world_str = "w";
constexpr const char* cust_str = "c";

CommunicationStore::CommunicationStore() {
    root.reset(VnV::Communication::create_mpi_communicator(VnVCommType::World));
    root->Initialize();
}


ICommunicator_ptr CommunicationStore::getCommunicator(VnV_Comm comm) {
  return root->custom(comm);
}

ICommunicator_ptr CommunicationStore::worldComm() {
  return root->world();
}

ICommunicator_ptr CommunicationStore::selfComm() {
  return root->self();
}


VnV_Comm CommunicationStore::world() {
  return worldComm()->get();
}
VnV_Comm CommunicationStore::self() {
  return selfComm()->get();
}

VnV_Comm CommunicationStore::custom(MPI_Comm comm) {
  ICommunicator_ptr p(VnV::Communication::create_mpi_communicator(comm));
  return p->get();
}


VnV_Comm CommunicationStore::toVnVComm(ICommunicator_ptr ptr) { return toVnVComm(ptr.get()); }

VnV_Comm CommunicationStore::toVnVComm(ICommunicator* ptr) { return ptr->get(); }


void CommunicationStore::Finalize() { root->Finalize(); }

BaseStoreInstance(CommunicationStore)

}  // namespace VnV
