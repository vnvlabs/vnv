#ifndef COMMUNICATIONSTORE_H
#define COMMUNICATIONSTORE_H

#include <type_traits>

#include "base/stores/BaseStore.h"
#include "common-interfaces/Communication.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IDataType.h"
using VnV::comm_register_ptr;
using VnV::ICommunicator;
using VnV::ICommunicator_ptr;

#include "validate/json-schema.hpp"

namespace VnV {

class CommunicationStore : public BaseStore {
  friend class Runtime;

 private:
  // Using long long here as the key because we want to be able to pass these
  // keys across processors in buffers. Long long avoids sending the char
  // arrays.

  ICommunicator_ptr root;
  std::string name;

  std::map<std::string, comm_register_ptr> communicator_factory;

 public:
  CommunicationStore();

  void addCommunicator(std::string name, comm_register_ptr factory);

  nlohmann::json schema(nlohmann::json& packageJson);

  void set(std::string name);

  VnV_Comm toVnVComm(ICommunicator_ptr ptr);
  VnV_Comm toVnVComm(ICommunicator* ptr);

  VnV_Comm custom(std::string packageName, void* data);

  VnV_Comm world();

  VnV_Comm self();

  ICommunicator_ptr getCommunicator(VnV_Comm comm);

  ICommunicator_ptr worldComm();

  ICommunicator_ptr selfComm();

  void Finalize();

  static CommunicationStore& instance();
};

}  // namespace VnV

#endif  // COMMUNICATIONSTORE_H
