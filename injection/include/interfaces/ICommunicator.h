﻿#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

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
  virtual IDataType_ptr reduce(IDataType_ptr &in, IDataType_ptr &out) = 0;
};



enum class SupportedDataType {
    DOUBLE, LONG, STRING, JSON
};

class PutData {
 public:
  std::string name;
  SupportedDataType datatype;
  std::vector<std::size_t> shape;
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
  virtual void axpy(double alpha, IDataType_ptr y) = 0;  // y = ax + y
  virtual int compare(IDataType_ptr y) = 0;  // -1 less, 0 == , 1 greater.
  virtual void mult(IDataType_ptr y) = 0;

  // Put will be called when you need to "Write" this output to file.
  // Put is called when someone type Put("name","datatype"). This is
  // collective on the Communicator used to type Put. So, this function
  // can output things like global vectors.
  // Put should call other Put actions
  virtual void Put(VnV::IOutputEngine* engine) = 0;


  // getLocalPutData is called when someone tries to write
  // a global vector containing objects of the this datatype
  // This is a "local" operation across the communicator set within
  // the engine.

  // This should be used to support writing vectors of a datatype. I.e., a
  // double vector. The Put is used to
  // support writing a (potentially distributed) data structure to file. This function
  // is used to support writing a distributed vector containing the data structure to
  // file.

  // The function should return a map of PutData objects that describe the different
  // parameters in the data structure.
  // THIS MUST BE CONSTANT ACROSS ALL INSTANCES OF THE DATA TYPE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // WE DO NOT SUPPORT DIFFERENT SIZES
  virtual std::map<std::string, PutData> getLocalPutData() = 0;

  // VnV will call this function when writing a distributed vector to file that contains
  // this data structure. The parameter name will be one of the keys returned in the getLocalPutData
  // map. The function should return a pointer to the data associated with this key.
  virtual void* getPutData(std::string name) = 0;

  void setKey(long long key);
  long long getKey();
  virtual ~IDataType();
};


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
  virtual void* getData() = 0; // Data is everything needed to rebuild this communicator.
  virtual void* raw() = 0; // Raw is a direct pointer to the comm a user of the internal proceedure might expect (e.g. MPI_Comm)

  virtual long uniqueId() = 0;  // Unique id such that the same comm data returns// the same comm.
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

  virtual void GatherV(void* buffer, int count, void* recvBuffer, int* recvCount, int* recvDispl, int dataTypeSize, int root) = 0;
  virtual void AllGatherV(void* buffer, int count, void* recvBuffer, int* recvCount, int* recvDispl, int dataTypeSize) = 0;


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

  virtual void Initialize();
  virtual void Finalize();

  template<typename T>
  T* getRaw() {
      return (T*) raw();
  }

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

template <typename T>
void registerDataType(std::string packageName,
                      VnV::Communication::dataType_ptr ptr) {
  std::string s = typeid(T).name();
  VnV::Communication::registerDataType(packageName, s, ptr);
}

}  // namespace Communication

}  // namespace VnV

#define INJECTION_COMM(PNAME, name)                                 \
  namespace VnV {                                                   \
  namespace PNAME {                                                 \
  namespace Communication {                                         \
  VnV::Communication::ICommunicator* declare_##name(                \
      VnV::Communication::CommType type);                           \
  void register_##name() {                                          \
    VnV::Communication::registerCommunicator(VNV_STR(PNAME), #name, \
                                             declare_##name);       \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  VnV::Communication::ICommunicator*                                \
      VnV::PNAME::Communication::declare_##name(                    \
          VnV::Communication::CommType type)

#define DECLARECOMM(PNAME, name) \
  namespace VnV {                \
  namespace PNAME {              \
  namespace Communication {      \
  void register_##name();        \
  }                              \
  }                              \
  }
#define REGISTERCOMM(PNAME, name) VnV::PNAME::Communication::register_##name();

#define INJECTION_DATATYPE(PNAME, name, cls)                                   \
  namespace VnV {                                                              \
  namespace PNAME {                                                            \
  namespace DataTypes {                                                        \
  VnV::Communication::IDataType* declare_##name();                             \
  void register_##name() {                                                     \
    VnV::Communication::registerDataType<cls>(VNV_STR(PNAME), declare_##name); \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  VnV::Communication::IDataType* VnV::PNAME::DataTypes::declare_##name()

#define DECLAREDATATYPE(PNAME, name) \
  namespace VnV {                    \
  namespace PNAME {                  \
  namespace DataTypes {              \
  void register_##name();            \
  }                                  \
  }                                  \
  }
#define REGISTERDATATYPE(PNAME, name) VnV::PNAME::DataTypes::register_##name();

#define INJECTION_REDUCER(PNAME, name, COMM)                     \
  namespace VnV {                                                \
  namespace PNAME {                                              \
  namespace Reducers {                                           \
    class name : public IReduction {                                 \
        virtual bool communitive() {return COMM; }                         \
        virtual IDataType_ptr reduce(IDataType_ptr &a, IDataType_ptr &b); \
                                                                 \
    };                                                           \
                                                                 \
    VnV::Communication::IReduction* declare_##name() {           \
         return new name();                                      \
    }                                                            \
    void register_##name() {                                     \
      VnV::Communication::registerReduction(VNV_STR(PNAME),      \
                                            #name,               \
                                            declare_##name);     \
  }                                                              \
  }                                                              \
  }                                                              \
  }                                                              \
  IDataType_ptr VnV::PNAME::Reducers::name::reduce(              \
      IDataType_ptr &in, IDataType_ptr &out                        \
  )



#define DECLAREREDUCER(PNAME, name) \
  namespace VnV {                   \
  namespace PNAME {                 \
  namespace Reducers {              \
  void register_##name();           \
  }                                 \
  }                                 \
  }

#define REGISTERREDUCER(PNAME, name) VnV::PNAME::Reducers::register_##name();

#endif  // ICOMMUNICATOR_H
