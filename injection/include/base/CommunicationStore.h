#ifndef COMMUNICATIONSTORE_H
#define COMMUNICATIONSTORE_H

#include <type_traits>

#include "interfaces/ICommunicator.h"

using VnV::Communication::ICommunicator;
using VnV::Communication::ICommunicator_ptr;
using VnV::Communication::IDataType;
using VnV::Communication::IDataType_ptr;
using VnV::Communication::IReduction;
using VnV::Communication::IReduction_ptr;
using VnV::Communication::CommType;
using VnV::Communication::comm_register_ptr;
using VnV::Communication::reduction_ptr;
using VnV::Communication::dataType_ptr;

namespace VnV {

class CommunicationStore {
 private:
  // Using long long here as the key because we want to be able to pass these
  // keys across processors in buffers. Long long avoids sending the char
  // arrays.
  std::map<long long, dataType_ptr> dataType_factory;
  std::map<long long, reduction_ptr> reduction_factory;
  std::map<long long, comm_register_ptr> communicator_factory;
  CommunicationStore();

  long long getKey(std::string packageName, std::string name);

 public:
  long long getKey(VnV::Communication::IDataType_ptr ptr);

  void addCommunicator(std::string packageName, std::string name,
                       comm_register_ptr factory);

  void addReduction(std::string packageName, std::string name, reduction_ptr m);

  void addDataType(std::string packageName, std::string name, dataType_ptr m);

  IDataType_ptr getDataType(long long key);

  IDataType_ptr getDataType(std::string pacakgeName, std::string name);

  IReduction_ptr getReducer(long long key);
  IReduction_ptr getReducer(std::string packageName, std::string name);

  ICommunicator_ptr getCommunicator(std::string packageName, std::string name,
                                    CommType type);

  ICommunicator_ptr getCommunicator(long long key, CommType type);

  static CommunicationStore& instance();
};



}  // namespace VnV

#endif  // COMMUNICATIONSTORE_H
