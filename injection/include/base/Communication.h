#ifndef COMMUNICATION_RUNTIME_H
#define COMMUNICATION_RUNTIME_H
#include "interfaces/ICommunicator.h"

namespace VnV {
namespace Communication {

class DataTypeCommunication {
  ICommunicator_ptr comm;

  IStatus_ptr Probe(int source, int tag);
  std::pair<IStatus_ptr, int> IProbe(int source, int tag);

  IStatus_ptr Wait(IRequest_ptr ptr);
  IStatus_vec WaitAll(IRequest_vec& vec);
  std::pair<IStatus_ptr, int> WaitAny(IRequest_vec& vec);
  std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr);
  std::pair<IStatus_vec, int> TestAll(IRequest_vec& vec);
  std::tuple<IStatus_ptr, int, int> TestAny(
      IRequest_vec& vec);

  IRequest_ptr Send(IDataType_vec& data, int dest, int tag, bool blocking);

  std::pair<IDataType_vec, IStatus_ptr> Recv(int count, long long dataType,
                                             int dest, int tag);

  // Recieve a data type when size and count are known.
  IRequest_ptr IRecv(int count, long long dataType, int dest, int tag);

  // Recv an arbitary data type without knowing what it is.
  std::pair<IDataType_vec, IStatus_ptr> Recv(int dest, int tag);

  IDataType_vec BroadCast(IDataType_vec& data, int count, int root,
                          bool allToAll);

  IDataType_vec Gather(IDataType_vec& data, int root, bool allGather);

  IDataType_vec Scatter(IDataType_vec& data, int root, int count);

  // TODO Assumes Datatype vec are all the same type.
  IDataType_vec Reduce(IDataType_vec& data, IReduction_ptr reduction, int root);
};

}  // namespace Communication
}  // namespace VnV
#endif  // COMMUNICATION_H
