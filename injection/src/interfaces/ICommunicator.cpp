#include "interfaces/ICommunicator.h"
#include "base/CommunicationStore.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include <iostream>

VnV::Communication::IRequest::~IRequest() {
  if (buffer) {
    free(buffer);
  }
}

VnV::Communication::IDataType_vec VnV::Communication::IRequest::unpack() {
  if (ready && buffer && recv) {
    long long* buff = (long long*)buffer;
    long long dataType = buff[0];
    IDataType_vec vec;
    IDataType_ptr ptrA = CommunicationStore::instance().getDataType(dataType);
    long dataSize = ptrA->maxSize();

    VnV::Communication::IDataType_vec results;
    char* cbuff = (char*)buffer;
    for (int i = 0; i < count; i++) {
      ptrA = CommunicationStore::instance().getDataType(dataType);
      long long* buff = (long long*)&(cbuff[i * dataSize]);
      ptrA->unpack(&(buff[1]));
      results.push_back(ptrA);
    }

    free(buffer);
    buffer = nullptr;
    return results;
  } else {
    throw VnV::VnVExceptionBase(
        "Attempted to unpack Request that was not marked ready or already "
        "unpacked or not a recv");
  }
}

void VnV::Communication::OpTypeEncodedReduction(void* invec, void* outvec,
                                                int* len) {
  long long* buff = (long long*)invec;
  long long reducerKey = buff[0];
  long long dataKey = buff[1];
  long dataSize =
      CommunicationStore::instance().getDataType(dataKey)->maxSize() +
      2 * sizeof(long long);
  IReduction_ptr reducer =
      CommunicationStore::instance().getReducer(reducerKey);

  // Call the reduction across all processors.
  char* cinvec = (char*)invec;
  char* coutvec = (char*)outvec;

  for (int i = 0; i < *len; i++) {
    buff = (long long*) &(cinvec[i * dataSize]);
    auto in = CommunicationStore::instance().getDataType(dataKey);
    in->unpack(&(buff[2]));

    buff = (long long*)&(coutvec[i * dataSize]);
    auto out = CommunicationStore::instance().getDataType(dataKey);
    out->unpack(&(buff[2]));



    out = reducer->reduce(in, out);
    out->pack(&(buff[2]));

    double* dd = (double*) &(buff[2]);
    std::cout << *dd << std::endl;

  }

  for (int i = 0; i < *len; i++) {
      buff = (long long*) &(coutvec[i * dataSize]);
      double* d = (double*) &(buff[2]);
      std::cout << buff[0] << " " << buff[1] << " " << *d << std::endl;;
  }

}
VnV::Communication::IStatus::~IStatus() {}

void VnV::Communication::ICommunicator::setPackage(std::string package) {
  packageName = package;
}


std::string VnV::Communication::ICommunicator::getPackage() {
  return packageName;
}

std::string VnV::Communication::ICommunicator::getName() {
   return keyName;
}
void VnV::Communication::ICommunicator::setName(std::string name) {
   keyName = name;
}

VnV_Comm VnV::Communication::ICommunicator::asComm() {
  return VnV_Create_Comm(getPackage().c_str(), getData());
}

long long VnV::Communication::IReduction::getKey() { return key; }

void VnV::Communication::IReduction::setKey(long long key) { this->key = key; }

void VnV::Communication::IDataType::setKey(long long key) { this->key = key; }

long long VnV::Communication::IDataType::getKey() { return key; }

VnV::Communication::IDataType::~IDataType() {}

void VnV::Communication::registerCommunicator(
    std::string packageName, std::string name,
    VnV::Communication::comm_register_ptr ptr) {
  VnV::CommunicationStore::instance().addCommunicator(packageName, name, ptr);
}

void VnV::Communication::registerReduction(
    std::string packageName, std::string name,
    VnV::Communication::reduction_ptr ptr) {
  CommunicationStore::instance().addReduction(packageName, name, ptr);
}

void VnV::Communication::registerDataType(
    std::string packageName, std::string name,
    VnV::Communication::dataType_ptr ptr) {
  VnV::CommunicationStore::instance().addDataType(packageName, name, ptr);
}
