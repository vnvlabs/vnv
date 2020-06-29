#ifndef COMMUNICATIONSTORE_H
#define COMMUNICATIONSTORE_H

#include <type_traits>

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/ICommunicator.h"

using VnV::Communication::comm_register_ptr;
using VnV::Communication::CommType;
using VnV::Communication::dataType_ptr;
using VnV::Communication::ICommunicator;
using VnV::Communication::ICommunicator_ptr;
using VnV::Communication::IDataType;
using VnV::Communication::IDataType_ptr;
using VnV::Communication::IReduction;
using VnV::Communication::IReduction_ptr;
using VnV::Communication::reduction_ptr;

namespace VnV {

class CommunicationStore {
 private:
  // Using long long here as the key because we want to be able to pass these
  // keys across processors in buffers. Long long avoids sending the char
  // arrays.

  std::map<std::string, std::pair<std::string, std::string>> commMap;
  std::map<long long, dataType_ptr> dataType_factory;
  std::map<long long, reduction_ptr> reduction_factory;
  std::map<long long, comm_register_ptr> communicator_factory;
  CommunicationStore();

  long long getKey(std::string name);
  long long getKey(std::string packageName, std::string name);
 public:
  void declareComm(std::string packageName, std::string commPackageName,
                   std::string commName);

  long long getKey(VnV::Communication::IDataType_ptr ptr);

  void addCommunicator(std::string packageName, std::string name,
                       comm_register_ptr factory);

  void addReduction(std::string packageName, std::string name, reduction_ptr m);

  void addDataType(std::string packageName, std::string name, dataType_ptr m);

  IDataType_ptr getDataType(long long key);

  IDataType_ptr getDataType(std::string name);

  template<typename T>
  IDataType_ptr getDataType() {
     return getDataType(typeid(T).name());
  }

  IReduction_ptr getReducer(long long key);
  IReduction_ptr getReducer(std::string packageName, std::string name);

  ICommunicator_ptr getCommunicator(std::string packageName, std::string name,
                                    CommType type);

  ICommunicator_ptr getCommunicator(long long key, CommType type);

  ICommunicator_ptr getCommForPackage(std::string packageName, CommType type);

  ICommunicator_ptr getCommunicator(VnV_Comm comm);

  ICommunicator_ptr worldComm(std::string packageName);

  ICommunicator_ptr selfComm(std::string packageName);

  ICommunicator_ptr customComm(std::string packageName, void* data);

  VnV_Comm toVnVComm(ICommunicator_ptr ptr);

  VnV_Comm customData(std::string packageName, void* data);

  VnV_Comm worldData(std::string packageName);

  VnV_Comm selfData(std::string packageName);

  static CommunicationStore& instance();
};

}  // namespace VnV

#endif  // COMMUNICATIONSTORE_H
