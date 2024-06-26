﻿#include "base/stores/CommunicationStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"

using VnV::CommType;

#ifdef WITH_MPI
#  define SUPP X(serial) X(mpi)
#  define DEFCOMM "mpi"
#else
#  define SUPP X(serial)
#  define DEFCOMM "serial"
#endif

namespace VnV {
namespace VNVPACKAGENAME {
namespace Communication {

#define X(a) VnV::ICommunicator* declare_##a(CommType type);
SUPP
#undef X

}  // namespace Communication
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

nlohmann::json CommunicationStore::schema(json& packageJson) {
  json j = R"({"type" : "string", "enum" : []})"_json;
  for (auto& it : communicator_factory) {
    j["enum"].push_back(it.first);
  }
  return j;
}

void CommunicationStore::set(std::string name) {
  if (name.empty()) {
    return set(DEFCOMM);
  }

  this->name = name;
  auto it = communicator_factory.find(name);
  if (it != communicator_factory.end()) {
    root.reset(it->second(CommType::World));
    root->setName(name);
    root->Initialize();
    return;
  }

  VnV_Error(VNVPACKAGENAME,
            "We could not find a communicator with the name %s. Please set the communication parameter in the \
    input file to a valid communicator class name. For now, we will use the default: %s",
            name.c_str(), DEFCOMM);

  // Use the default
  set("");
}

void CommunicationStore::addCommunicator(std::string name, comm_register_ptr m) {
  communicator_factory.insert(std::make_pair(name, m));
}

ICommunicator_ptr CommunicationStore::getCommunicator(VnV_Comm comm) {
  if (comm.name == world_str) {
    return worldComm();
  } else if (comm.name == self_str) {
    return selfComm();
  } else if (comm.name == cust_str) {
    auto p = root->custom(comm.data, false);
    p->setName(name);
    return p;
  }
  throw INJECTION_BUG_REPORT("Invalid name in VnV_Comm %s", comm.name);
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

VnV_Comm CommunicationStore::toVnVComm(ICommunicator_ptr ptr) { return toVnVComm(ptr.get()); }

VnV_Comm CommunicationStore::toVnVComm(ICommunicator* ptr) { return {cust_str, ptr->getData()}; }

VnV_Comm CommunicationStore::custom(std::string commName, void* data) {
  if (commName == name) {
    return {cust_str, root->custom(data, true)->getData()};
  } else {
    auto p = root->handleOtherCommunicators(commName, data);
    return {cust_str, p->getData()};
  }
}

VnV_Comm CommunicationStore::world() { return {world_str, NULL}; }

VnV_Comm CommunicationStore::self() { return {self_str, NULL}; }


void CommunicationStore::Finalize() { root->Finalize(); }

BaseStoreInstance(CommunicationStore)

}  // namespace VnV
