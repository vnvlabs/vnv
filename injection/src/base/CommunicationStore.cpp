﻿#include "base/CommunicationStore.h"
#include <iostream>

#include "base/Utilities.h"
#include "base/exceptions.h"

using VnV::Communication::CommType;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Communication {
VnV::Communication::ICommunicator* declare_serial(CommType type);
}
}  // namespace VNVPACKAGENAME
}  // namespace VnV

namespace VnV {

CommunicationStore::CommunicationStore() {
  // Add the serial communicator on construction. This ensures the serial comm
  // is always available. This is needed by debug statements that occur before
  // plugin registration.
  addCommunicator(VNV_STR(VNVPACKAGENAME), "serial",
                  VnV::VNVPACKAGENAME::Communication::declare_serial);
}

long long CommunicationStore::getKey(std::string packageName,
                                     std::string name) {
  return VnV::StringUtils::simpleHash(packageName + ":" + name);
}

long long CommunicationStore::getKey(std::string name) {
  return getKey("", name);
}

void CommunicationStore::declareComm(std::string packageName,
                                     std::string commPackageName,
                                     std::string commName) {
  commMap[packageName] = {commPackageName, commName};
}

long long CommunicationStore::getKey(IDataType_ptr ptr) {
  return ptr->getKey();
}

void CommunicationStore::addCommunicator(std::string packageName,
                                         std::string name,
                                         comm_register_ptr m) {
  m(CommType::World)->Initialize();
  communicator_factory.insert(std::make_pair(getKey(packageName, name), m));
}

void CommunicationStore::addReduction(std::string packageName, std::string name,
                                      reduction_ptr m) {
  reduction_factory.insert(std::make_pair(getKey(packageName, name), m));
}

void CommunicationStore::addDataType(std::string packageName, std::string name,
                                     dataType_ptr m) {
  dataType_factory.insert(std::make_pair(getKey(name), m));
}

IDataType_ptr CommunicationStore::getDataType(long long key) {
  auto it = dataType_factory.find(key);
  if (it != dataType_factory.end()) {
    IDataType_ptr ptr(it->second());
    ptr->setKey(key);
    return ptr;
  }
  throw VnV::VnVExceptionBase("Unsupported Data Type");
}

IDataType_ptr CommunicationStore::getDataType(std::string name) {
  return getDataType(getKey(name));
}


IReduction_ptr CommunicationStore::getReducer(long long key) {
  auto it = reduction_factory.find(key);
  if (it != reduction_factory.end()) {
    IReduction_ptr s(it->second());
    s->setKey(key);
    return s;
  }
  throw VnV::VnVExceptionBase("Un supported Data Type)");
}

IReduction_ptr CommunicationStore::getReducer(std::string packageName,
                                              std::string name) {
  return getReducer(getKey(packageName, name));
}
IReduction_ptr CommunicationStore::getReducer(std::string packageColonName) {
  return getReducer(VnV::StringUtils::simpleHash(packageColonName));
}

ICommunicator_ptr CommunicationStore::getCommunicator(std::string packageName,
                                                      std::string name,
                                                      CommType type) {
  long long key = getKey(packageName, name);
  auto it = communicator_factory.find(key);
  if (it != communicator_factory.end()) {
    ICommunicator_ptr t(it->second(type));
    t->setName(packageName + ":" + name);
    return t;
  }
  throw VnV::VnVExceptionBase("Un supported Data Type)");
}


Communication::ICommunicator_ptr CommunicationStore::getCommForPackage(
    std::string packageName, Communication::CommType type) {
  auto it = commMap.find(packageName);
  if (it != commMap.end()) {
    auto c = getCommunicator(it->second.first, it->second.second, type);
    c->setPackage(packageName);


    return c;
  }

  // RETURN THE DEFAULT PACKAGE NAME.
  auto c = getCommunicator(VNV_STR(VNVPACKAGENAME), "serial", type);
  c->setPackage(VNV_STR(VNVPACKAGENAME));
  return c;
}
Communication::ICommunicator_ptr CommunicationStore::getCommunicator(
    VnV_Comm comm) {
  return customComm(comm.name, comm.data);
}

Communication::ICommunicator_ptr CommunicationStore::worldComm(
    std::string packageName) {
  return getCommForPackage(packageName, CommType::World);
}

Communication::ICommunicator_ptr CommunicationStore::selfComm(
    std::string packageName) {
  return getCommForPackage(packageName, CommType::Self);
}

Communication::ICommunicator_ptr CommunicationStore::customComm(
    std::string packageName, void* data) {
  auto c = getCommForPackage(packageName, CommType::Default);
  c->setData(data);
  return c;
}

VnV_Comm CommunicationStore::toVnVComm(Communication::ICommunicator_ptr ptr) {
  return customData(ptr->getPackage(), ptr->getData());
}

VnV_Comm CommunicationStore::customData(std::string packageName, void* data) {
  return VnV_Create_Comm(packageName.c_str(), data);
}

VnV_Comm CommunicationStore::worldData(std::string packageName) {
  return VnV_Create_Comm(packageName.c_str(),
                         worldComm(packageName)->getData());
}

VnV_Comm CommunicationStore::selfData(std::string packageName) {
  return VnV_Create_Comm(packageName.c_str(), selfComm(packageName)->getData());
}

CommunicationStore& CommunicationStore::instance() {
  static CommunicationStore store;
  return store;
}
void CommunicationStore::Finalize() {
  for (const auto& item : communicator_factory) {
      item.second(CommType::World)->Finalize();
  }
}

}  // namespace VnV
