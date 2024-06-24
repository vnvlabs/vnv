#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common-interfaces/Communication.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/IDataType.h"

namespace VnV {

class IOutputEngine;

class IRequest {
 public:
  void* buffer;
  int count;
  bool recv = false;
  bool ready = false;

  IDataType_vec unpack();

  virtual ~IRequest();
};
typedef std::shared_ptr<IRequest> IRequest_ptr;
typedef std::vector<IRequest_ptr> IRequest_vec;

class IStatus {
 public:
  virtual int source() = 0;
  virtual int tag() = 0;
  virtual int error() = 0;
  virtual ~IStatus();
};
typedef std::shared_ptr<IStatus> IStatus_ptr;
typedef std::vector<IStatus_ptr> IStatus_vec;

// Define an interface for Reduction operations. The implementation
// should support all operations in the optype. When the type is
// encoded, the implementation should use a Operation can calls
// the OpTypeEncodedReduction function.
enum class OpType { MIN, MAX, SUM, PROD, LAND, LOR, ENCODED_COMMUTE, ENCODED_NONCOMMUTE };
void OpTypeEncodedReduction(void* invec, void* outvec, int* len);

class ICommunicator;
typedef std::shared_ptr<ICommunicator> ICommunicator_ptr;

// DataType -- This is a type that can be put into a

// Distributed Data Type --> Data Type that can be put into a global array.

// Enum class to describe the different return types for a comm compare.
// Exact means the comms have same contexts and groups
// Group means different contexts but same groups
// SIMILAR means different contexts but similar (same proces different order)
// groups UN equal -- not the same.
enum class CommCompareType { EXACT, GROUP, SIMILAR, UNEQUAL };

class ICommunicator {
 private:
  std::string packageName;
  std::string keyName;

 public:
  void setPackage(std::string package);
  std::string getPackage();
  std::string getName();
  void setName(std::string name);

  VnV_Comm asComm();

  virtual ~ICommunicator() {}

  virtual void setData(void* data) = 0;  // The communicator passed in.
  virtual void* getData() = 0;           // Data is everything needed to rebuild this communicator.
  virtual void* raw() = 0;               // Raw is a direct pointer to the comm a user of the
                                         // internal proceedure might expect (e.g. MPI_Comm)

  virtual long uniqueId() = 0;  // Unique id such that the same comm data
                                // returns// the same comm.
  virtual int Size() = 0;
  virtual int Rank() = 0;
  virtual void Barrier() = 0;
  virtual std::string ProcessorName() = 0;
  virtual double time() = 0;
  virtual double tick() = 0;

  virtual ICommunicator_ptr duplicate() = 0;
  virtual ICommunicator_ptr split(int color, int key) = 0;
  virtual ICommunicator_ptr create(std::vector<int>& ranks, int tag) = 0;
  virtual ICommunicator_ptr create(int start, int end, int stride, int tag) = 0;
  virtual ICommunicator_ptr world() = 0;
  virtual ICommunicator_ptr self() = 0;
  virtual ICommunicator_ptr custom(void* data, bool raw) = 0;

  virtual ICommunicator_ptr handleOtherCommunicators(std::string name, void* data) = 0;

  virtual CommCompareType compare(ICommunicator_ptr ptr) = 0;
  virtual bool contains(ICommunicator_ptr) = 0;
  virtual bool contains(long proc) = 0;


  virtual void Send(void* buffer, int count, int dest, int tag, int dataTypeSize) = 0;
  virtual IRequest_ptr ISend(void* buffer, int count, int dest, int tag, int dataTypeSize) = 0;

  virtual IStatus_ptr Recv(void* buffer, int count, int dest, int tag, int dataTypeSize) = 0;
  virtual IRequest_ptr IRecv(void* buffer, int count, int dest, int tag, int dataTypeSize) = 0;

  virtual IStatus_ptr Wait(IRequest_ptr ptr) = 0;
  virtual IStatus_vec WaitAll(IRequest_vec& vec) = 0;
  virtual std::pair<IStatus_ptr, int> WaitAny(IRequest_vec& vec) = 0;

  virtual int Count(IStatus_ptr status, int dataTypeSize) = 0;
  virtual IStatus_ptr Probe(int source, int tag) = 0;
  virtual std::pair<IStatus_ptr, int> IProbe(int source, int tag) = 0;

  virtual std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr) = 0;
  virtual std::pair<IStatus_vec, int> TestAll(IRequest_vec& vec) = 0;
  virtual std::tuple<IStatus_ptr, int, int> TestAny(IRequest_vec& vec) = 0;

  virtual void Gather(void* buffer, int count, void* recvBuffer, int dataTypeSize, int root) = 0;

  virtual void AllGather(void* buffer, int count, void* recvBuffer, int dataTypeSize) = 0;

  virtual void GatherV(void* buffer, int count, void* recvBuffer, int* recvCount, int* recvDispl, int dataTypeSize,
                       int root) = 0;
  virtual void AllGatherV(void* buffer, int count, void* recvBuffer, int* recvCount, int* recvDispl,
                          int dataTypeSize) = 0;

  virtual void BroadCast(void* buffer, int count, int dataTypeSize, int root) = 0;
  virtual void AllToAll(void* buffer, int count, void* recvBuffer, int dataTypeSize) = 0;
  virtual void Scatter(void* buffer, int count, void* recvBuffer, int dataTypeSize, int root) = 0;

  virtual void Reduce(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op, int root) = 0;
  virtual void AllReduce(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op) = 0;
  virtual void Scan(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op) = 0;

  virtual void Abort(int errorcode) = 0;

  virtual void Initialize();
  virtual void Finalize();

  virtual int VersionMajor() = 0;
  virtual int VersionMinor() = 0;
  virtual std::string VersionLibrary() = 0;

  template <typename T> T* getRaw() { return (T*)raw(); }
};

enum class CommType { World, Self, Default };

typedef ICommunicator* (*comm_register_ptr)(CommType);

}  // namespace VnV

#define INJECTION_COMM(PNAME, name)                       \
  namespace VnV {                                         \
  namespace PNAME {                                       \
  namespace Communication {                               \
  VnV::ICommunicator* declare_##name(VnV::CommType type); \
  }                                                       \
  }                                                       \
  }                                                       \
  VnV::ICommunicator* VnV::PNAME::Communication::declare_##name(VnV::CommType type)

#endif  // ICOMMUNICATOR_H
