#include "base/CommunicationStore.h"
#include "base/Utilities.h"
#include "base/exceptions.h"

namespace VnV {

CommunicationStore::CommunicationStore() {}

long long CommunicationStore::getKey(std::string packageName,
                                     std::string name) {
  return VnV::StringUtils::simpleHash(packageName + ":" + name);
}

long long CommunicationStore::getKey(IDataType_ptr ptr) {
  return ptr->getKey();
}

void CommunicationStore::addCommunicator(std::string packageName,
                                         std::string name, comm_factory m) {
  communicator_factory.insert(std::make_pair(getKey(packageName, name), m));
}

void CommunicationStore::addReduction(std::string packageName, std::string name,
                                      reduction_ptr m) {
  reduction_factory.insert(std::make_pair(getKey(packageName, name), m));
}

void CommunicationStore::addDataType(std::string packageName, std::string name,
                                     datatype_ptr m) {
  dataType_factory.insert(std::make_pair(getKey(packageName, name), m));
}

IDataType_ptr CommunicationStore::getDataType(long long key) {
  auto it = dataType_factory.find(key);
  if (it != dataType_factory.end()) {
    IDataType_ptr ptr(it->second());
    ptr->setKey(key);
    return ptr;
  }
  throw VnV::VnVExceptionBase("Un supported Data Type");
}

IDataType_ptr CommunicationStore::getDataType(std::string packageName,
                                              std::string name) {
  return getDataType(getKey(packageName, name));
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

ICommunicator_ptr CommunicationStore::getCommunicator(std::string packageName,
                                                      std::string name,
                                                      CommType type) {
  return getCommunicator(getKey(packageName, name), type);
}

ICommunicator_ptr CommunicationStore::getCommunicator(long long key,
                                                      CommType type) {
  auto it = communicator_factory.find(key);
  if (it != communicator_factory.end()) {
    return ICommunicator_ptr(it->second(type));
  }
  throw VnV::VnVExceptionBase("Un supported Data Type)");
}

CommunicationStore& CommunicationStore::instance() {
  static CommunicationStore store;
  return store;
}

}  // namespace VnV
