#ifndef COMMUNICATION_RUNTIME_H
#define COMMUNICATION_RUNTIME_H
#include "base/stores/CommunicationStore.h"
#include "base/stores/ReductionStore.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IDataType.h"
#include "interfaces/IReduction.h"

namespace VnV {

class DataTypeCommunication {
  ICommunicator_ptr comm;

 public:
  DataTypeCommunication(ICommunicator_ptr ptr) : comm(ptr) {}

  IStatus_ptr Probe(int source, int tag);
  std::pair<IStatus_ptr, int> IProbe(int source, int tag);

  IStatus_ptr Wait(IRequest_ptr ptr);
  IStatus_vec WaitAll(IRequest_vec& vec);
  std::pair<IStatus_ptr, int> WaitAny(IRequest_vec& vec);
  std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr);
  std::pair<IStatus_vec, int> TestAll(IRequest_vec& vec);
  std::tuple<IStatus_ptr, int, int> TestAny(IRequest_vec& vec);

  IRequest_ptr Send(IDataType_vec& data, int dest, int tag, bool blocking);

  std::pair<IDataType_vec, IStatus_ptr> Recv(int count, long long dataType,
                                             int dest, int tag);

  // Recieve a data type when size and count are known.
  IRequest_ptr IRecv(int count, long long dataType, int dest, int tag);

  // Recv an arbitary data type without knowing what it is.
  std::pair<IDataType_vec, IStatus_ptr> Recv(int dest, int tag);

  IDataType_vec BroadCast(IDataType_vec& data, long long dtype, int count,
                          int root, bool allToAll);

  IDataType_vec Gather(IDataType_vec& data, long long dtype, int root,
                       bool allGather);

  IDataType_vec GatherV(IDataType_vec& data, long long dtype, int root,
                        std::vector<int>& gsizes, std::vector<int>& sizes,
                        std::vector<int>& offsets, bool allGather);

  IDataType_vec Scatter(IDataType_vec& data, long long dtype, int root,
                        int count);

  // TODO Assumes Datatype vec are all the same type.

  // NOTE: Each data element is considered a new reduction variable. I.e, this
  // assumes every process puts in the same number of data points. To reduce a
  // "vector" where each process has multiple entities, make sure to set
  // "vector=true"!
  IDataType_vec ReduceMultiple(IDataType_vec& data, long long dtype,
                               IReduction_ptr reduction, int root);
  IDataType_vec ReduceMultiple(IDataType_vec& data, long long dtype,
                               std::string reducer, int root);

  IDataType_ptr ReduceVector(IDataType_vec& data, long long dtype,
                             std::string reducer, int root) {
    return ReduceVector(data, dtype,
                        ReductionStore::instance().getReducer(reducer), root);
  }

  IDataType_ptr ReduceVector(IDataType_vec& data, long long dtype,
                             IReduction_ptr reducer, int root) {
    IDataType_vec v = {ReduceLocalVec(data, reducer)};
    auto r = ReduceMultiple(v, dtype, reducer, root);
    return (r.size() == 1) ? r[0] : nullptr;
  }

  IDataType_ptr ReduceLocalVec(IDataType_vec& data, IReduction_ptr reducer);

  IDataType_ptr ReduceLocalVec(IDataType_vec& data, std::string reducer) {
    return ReduceLocalVec(data, ReductionStore::instance().getReducer(reducer));
  }
};

}  // namespace VnV
#endif  // COMMUNICATION_H
