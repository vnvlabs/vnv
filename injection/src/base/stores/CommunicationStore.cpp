#include "base/stores/CommunicationStore.h"
#include <iostream>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/Runtime.h"

using VnV::CommType;

#ifdef WITH_MPI
#define SUPP X(serial) X(mpi)
#define DEFCOMM "mpi"
#else
#define SUPP X(serial)
#define DEFCOMM "serial"
#endif


namespace VnV {
namespace VNVPACKAGENAME {
namespace Communication {

#define X(a) VnV::ICommunicator* declare_##a(CommType type);
SUPP
#undef X

}
}  // namespace VNVPACKAGENAME
}  // namespace VnV

namespace VnV {

constexpr const char* self_str = "s";
constexpr const char* world_str = "w";
constexpr const char* cust_str = "c";


CommunicationStore::CommunicationStore() {   
  #define X(a) addCommunicator(#a, VnV::VNVPACKAGENAME::Communication::declare_##a);
  SUPP
  #undef X
}

void CommunicationStore::set(std::string name)  {
    if (name.empty()) {
      return set(DEFCOMM);
    }
    
    this->name = name;

    auto it = communicator_factory.find(name);
    if (it != communicator_factory.end()) {
      root.reset( it->second(CommType::World) );
      root->setName(name);
      root->Initialize();
      return;
    }

    throw VnVExceptionBase("We could not find a communicator with the name %s. Please set the communication parameter in the \
    input file to a valid communicator class name. The default is %s", name.c_str(), DEFCOMM);

}

void CommunicationStore::addCommunicator(std::string name,
                                         comm_register_ptr m) {

  communicator_factory.insert(std::make_pair(name, m));

}

ICommunicator_ptr CommunicationStore::getCommunicator(VnV_Comm comm) {
  
  if (comm.name == world_str ) {
    return worldComm();
  } else if (comm.name == self_str) {
    return selfComm();
  } else if (comm.name == cust_str) {  
    auto p = root->custom(comm.data);  
    p->setName(name);
    return p;
  }
  throw VnVExceptionBase("Invalid package name"); 
}


ICommunicator_ptr CommunicationStore::worldComm() {
  auto p = root->world();
  p->setName(name);
  return p;
}

ICommunicator_ptr CommunicationStore::selfComm() {
  auto p = root->self();
  p->setName(name);
  return p;
  
}

VnV_Comm CommunicationStore::toVnVComm(ICommunicator_ptr ptr) {
  return toVnVComm(ptr.get());
}

VnV_Comm CommunicationStore::toVnVComm(ICommunicator* ptr) {
  return {cust_str, ptr->getData()};
}

VnV_Comm CommunicationStore::custom(std::string commName, void* data) {
  if (commName == name) {
     return {cust_str, data};
  } else {
     auto p = root->handleOtherCommunicators(commName, data);
     return {cust_str, p->getData()};
  }
  throw VnVExceptionBase("Invalid Communication Objects");
}

VnV_Comm CommunicationStore::world() {
  return {world_str,NULL};
}

VnV_Comm CommunicationStore::self() {
  return {self_str,NULL};
}

void CommunicationStore::Finalize() {
  root->Finalize();
}

BaseStoreInstance(CommunicationStore)

}  // namespace VnV
