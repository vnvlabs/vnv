﻿#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <map>
#include <memory>
#include <vector>
#include <string>

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"

namespace VnV {
class IOutputEngine;
namespace Communication {

class IDataType;
typedef std::shared_ptr<IDataType> IDataType_ptr;
typedef std::vector<IDataType_ptr> IDataType_vec;

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
enum class OpType {
  MIN,
  MAX,
  SUM,
  PROD,
  LAND,
  LOR,
  ENCODED_COMMUTE,
  ENCODED_NONCOMMUTE
};
void OpTypeEncodedReduction(void* invec, void* outvec, int* len);

class ICommunicator;
typedef std::shared_ptr<ICommunicator> ICommunicator_ptr;

class IReduction;
typedef std::shared_ptr<IReduction> IReduction_ptr;
typedef std::vector<IReduction_ptr> IReduction_vec;

class IReduction {
 private:
  long long key;

 public:
  virtual ~IReduction() {}

  long long getKey();
  void setKey(long long key);

  virtual bool communitive() = 0;  // Is this communative.
  virtual IDataType_ptr reduce(IDataType_ptr in, IDataType_ptr out) = 0;
};

class IDataType {
 private:
  long long key;

 public:
  virtual long long maxSize() = 0;  // what is the maximum size of the buffer
                                    // defined in this class.
  virtual long long pack(void* buffer) = 0;  // pack the buffer
  virtual void unpack(void* buffer) = 0;     // unpack into a buffer
  virtual void setData(void* data) = 0;  // set from a raw pointer to the data.
  virtual void axpy(double alpha, IDataType* y) = 0;  // y = ax + y
  virtual int compare(IDataType* y) = 0;  // -1 less, 0 == , 1 greater.
  virtual void mult(IDataType* y) = 0;
  virtual void Put(VnV_Comm comm, IOutputEngine *engine) = 0;

  void setKey(long long key);
  long long getKey();

  virtual ~IDataType();
};

// Enum class to describe the different return types for a comm compare.
// Exact means the comms have same contexts and groups
// Group means different contexts but same groups
// SIMILAR means different contexts but similar (same proces different order)
// groups UN equal -- not the same.
enum class CommCompareType { EXACT, GROUP, SIMILAR, UNEQUAL };

class ICommunicator {
 private:
  std::string packageName;

 public:
  void setPackage(std::string package);
  std::string getPackage();
  VnV_Comm asComm();

  virtual ~ICommunicator() {}

  virtual void setData(void* data) = 0;  // The communicator passed in.
  virtual void* getData() = 0;

  virtual int uniqueId() = 0;  // Unique id such that the same comm data returns
                               // the same comm.

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

  virtual CommCompareType compare(ICommunicator_ptr ptr) = 0;
  virtual bool contains(ICommunicator_ptr) = 0;

  virtual void Send(void* buffer, int count, int dest, int tag,
                    int dataTypeSize) = 0;
  virtual IRequest_ptr ISend(void* buffer, int count, int dest, int tag,
                             int dataTypeSize) = 0;

  virtual IStatus_ptr Recv(void* buffer, int count, int dest, int tag,
                           int dataTypeSize) = 0;
  virtual IRequest_ptr IRecv(void* buffer, int count, int dest, int tag,
                             int dataTypeSize) = 0;

  virtual IStatus_ptr Wait(IRequest_ptr ptr) = 0;
  virtual IStatus_vec WaitAll(IRequest_vec& vec) = 0;
  virtual std::pair<IStatus_ptr, int> WaitAny(IRequest_vec& vec) = 0;

  virtual int Count(IStatus_ptr status, int dataTypeSize) = 0;
  virtual IStatus_ptr Probe(int source, int tag) = 0;
  virtual std::pair<IStatus_ptr, int> IProbe(int source, int tag) = 0;

  virtual std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr) = 0;
  virtual std::pair<IStatus_vec, int> TestAll(IRequest_vec& vec) = 0;
  virtual std::tuple<IStatus_ptr, int, int> TestAny(IRequest_vec& vec) = 0;

  virtual void Gather(void* buffer, int count, void* recvBuffer,
                      int dataTypeSize, int root) = 0;
  virtual void AllGather(void* buffer, int count, void* recvBuffer,
                         int dataTypeSize) = 0;

  virtual void BroadCast(void* buffer, int count, int dataTypeSize,
                         int root) = 0;
  virtual void AllToAll(void* buffer, int count, void* recvBuffer,
                        int dataTypeSize) = 0;
  virtual void Scatter(void* buffer, int count, void* recvBuffer,
                       int dataTypeSize, int root) = 0;

  virtual void Reduce(void* buffer, int count, void* recvBuffer,
                      int dataTypeSize, OpType op, int root) = 0;
  virtual void AllReduce(void* buffer, int count, void* recvBuffer,
                         int dataTypeSize, OpType op) = 0;
  virtual void Scan(void* buffer, int count, void* recvBuffer, int dataTypeSize,
                    OpType op) = 0;

  virtual void Abort(int errorcode) = 0;
};

enum class CommType { World, Self, Default };

typedef ICommunicator* (*comm_register_ptr)(CommType);
typedef IReduction* (*reduction_ptr)();
typedef IDataType* (*dataType_ptr)();




void registerCommunicator(std::string packageName, std::string name,
                          VnV::Communication::comm_register_ptr r);
void registerDataType(std::string packageName, std::string name,
                      VnV::Communication::dataType_ptr r);
void registerReduction(std::string packageName, std::string name,
                       VnV::Communication::reduction_ptr r);

template<typename T>
void registerDataType(std::string packageName, VnV::Communication::dataType_ptr ptr) {
   std::string s = typeid(T).name();
   VnV::Communication::registerDataType(packageName, s,ptr);
}

}  // namespace Communication

}  // namespace VnV

#define INJECTION_COMM(name)                                        \
  namespace VnV {                                                   \
  namespace PACKAGENAME {                                           \
  namespace Communication {                                         \
  VnV::Communication::ICommunicator* declare_##name(CommType type); \
  void register_##name() {                                          \
    VnV::Communication::registerCommunicator(PACKAGENAME_S, #name, declare_##name);       \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  VnV::Communication::ICommunicator*                                \
      VnV::PACKAGENAME::Communication::declare_##name(CommType type)

#define DECLARECOMM(name)   \
  namespace VnV {           \
  namespace PACKAGENAME {   \
  namespace Communication { \
  void register_##name();   \
  }                         \
  }                         \
  }
#define REGISTERCOMM(name) VnV::PACKAGENAME::Communication::register_##name();

#define INJECTION_DATATYPE(cls)                               \
  namespace VnV {                                              \
  namespace PACKAGENAME {                                      \
  namespace DataTypes {                                        \
  VnV::Communication::IDataType* declare_##name();             \
  void register_##name() {                                     \
    VnV::Communication::registerDataType<cls>(PACKAGENAME_S, declare_##name); \
  }                                                            \
  }                                                            \
  }                                                            \
  }                                                            \
  VnV::Communication::IDataType* VnV::PACKAGENAME::DataTypes::declare_##name()

#define DECLAREDATATYPE(name) \
  namespace VnV {             \
  namespace PACKAGENAME {     \
  namespace DataTypes {       \
  void register_##name();     \
  }                           \
  }                           \
  }
#define REGISTERDATATYPE(name) VnV::PACKAGENAME::DataTypes::register_##name();

#define INJECTION_REDUCER(name)                                 \
  namespace VnV {                                               \
  namespace PACKAGENAME {                                       \
  namespace Reducers {                                          \
  VnV::Communication::IReduction* declare_##name();             \
  void register_##name() {                                      \
    VnV::Communication::registerReduction(PACKAGENAME_S, #name, \
                                          declare_##name);      \
  }                                                             \
  }                                                             \
  }                                                             \
  }                                                             \
  VnV::Communication::IReduction* VnV::PACKAGENAME::Reducers::declare_##name()

#define DECLAREREDUCER(name) \
  namespace VnV {            \
  namespace PACKAGENAME {    \
  namespace Reducers {       \
  void register_##name();    \
  }                          \
  }                          \
  }
#define REGISTERREDUCER(name) VnV::PACKAGENAME::Reducers::register_##name();

#endif  // ICOMMUNICATOR_H
