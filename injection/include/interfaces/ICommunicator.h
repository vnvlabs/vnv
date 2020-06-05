#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <memory>
#include <vector>
#include <map>


namespace VnV {
namespace Communication {

class IDataType;
typedef std::shared_ptr<IDataType> IDataType_ptr;
typedef std::vector<IDataType_ptr> IDataType_vec;

class IRequest {
public:
    void* buffer;
    int count;
    bool ready = false;

    virtual ~IRequest();
};
typedef std::shared_ptr<IRequest> IRequest_ptr;
typedef std::vector<IRequest_ptr> IRequest_vec;

class IRecvRequest : public IRequest {
public:
    IDataType_vec unpack();
    virtual ~IRecvRequest() = default;
};
typedef std::shared_ptr<IRecvRequest> IRecvRequest_ptr;
typedef std::vector<IRecvRequest_ptr> IRecvRequest_vec;


class ISendRequest : public IRequest {
    virtual ~ISendRequest() = default;
};
typedef std::shared_ptr<ISendRequest> ISendRequest_ptr;
typedef std::vector<ISendRequest_ptr> ISendRequest_vec;


class IStatus {
public:
    virtual int source() = 0;
    virtual int tag() = 0;
    virtual int error() = 0;
    virtual ~IStatus() = default;
};
typedef std::shared_ptr<IStatus> IStatus_ptr;
typedef std::vector<IStatus_ptr> IStatus_vec;

// Define an interface for Reduction operations. The implementation
// should support all operations in the optype. When the type is
// encoded, the implementation should use a Operation can calls
// the OpTypeEncodedReduction function.
enum class OpType { MIN, MAX, SUM, PROD, LAND, LOR, ENCODED_COMMUTE, ENCODED_NONCOMMUTE };
static void OpTypeEncodedReduction(void* invec, void* outvec, int* len);

class ICommunicator;
typedef std::shared_ptr<ICommunicator> ICommunicator_ptr;


class IReduction;
typedef std::shared_ptr<IReduction> IReduction_ptr;
typedef std::vector<IReduction_ptr> IReduction_vec;


class IReduction {
private:
    long long key;
public:
    long long getKey();
    void setKey(long long key);

    virtual bool communitive() = 0; // Is this communative.
    virtual IDataType_ptr reduce(IDataType_ptr in, IDataType_ptr out) = 0;
};

class IDataType {
private:
    long long key;
public:
    virtual long long maxSize() = 0; // what is the maximum size of the buffer defined in this class.
    virtual long long pack( void* buffer) = 0; // pack the buffer
    virtual void unpack(void* buffer) = 0; // unpack into a buffer
    virtual void setData(void* data) = 0; // set from a raw pointer to the data.
    virtual void axpy(double alpha, IDataType *y) = 0; // y = ax + y
    virtual int compare(IDataType *y) = 0 ; // -1 less, 0 == , 1 greater.

    void setKey(long long key);
    long long getKey();

    virtual ~IDataType();
};

class ICommunicator {
public:
    virtual int  Size() = 0;
    virtual int  Rank() = 0;
    virtual void Barrier() = 0;
    virtual std::string  ProcessorName()=0;
    virtual double time()=0;
    virtual double tick()=0;

    virtual ICommunicator_ptr duplicate()=0;
    virtual ICommunicator_ptr split(int color, int key) = 0;
    virtual ICommunicator_ptr create(std::vector<int> &ranks, int stride) = 0;
    virtual ICommunicator_ptr create(int start, int end, int stride, int tag) = 0;

    virtual void Send(void*buffer, int count, int dest, int tag, int dataTypeSize) = 0;
    virtual ISendRequest_ptr ISend(void*buffer, int count, int dest, int tag, int dataTypeSize) = 0;

    virtual IStatus_ptr Recv( void* buffer, int count, int dest, int tag, int dataTypeSize) = 0;
    virtual IRecvRequest_ptr IRecv( void*buffer, int count, int dest, int tag, int dataTypeSize) = 0;

    virtual IStatus_ptr Wait(IRequest_ptr ptr) = 0;
    virtual IStatus_vec WaitAll(IRequest_vec &vec) = 0;
    virtual std::pair<IStatus_vec, int> WaitAny(IRequest_vec &vec) = 0;

    virtual int Count(IStatus_ptr status, int dataTypeSize) = 0;
    virtual IStatus_ptr Probe(int source, int tag) = 0;
    virtual std::pair<IStatus_ptr,int> IProbe(int source, int tag) = 0;

    virtual std::pair<IStatus_ptr,int> Test(IRequest_ptr ptr) = 0;
    virtual std::vector<std::pair<IStatus_ptr,int>> TestAll(IRequest_vec &vec) = 0;
    virtual std::pair<std::vector<std::pair<IStatus_ptr, int>>,int> TestAny(IRequest_vec &vec) = 0;

    virtual void Gather(void* buffer, int count, void* recvBuffer, int dataTypeSize, int root)= 0;
    virtual void AllGather(void* buffer, int count, void* recvBuffer, int dataTypeSize)= 0;

    virtual void BroadCast(void* buffer, int count, void*recvBuffer, int dataTypeSize, int root)= 0 ;
    virtual void AllToAll(void* buffer, int count, void*recvBuffer, int dataTypeSize)= 0 ;
    virtual void Scatter(void* buffer, int count, void* recvBuffer, int dataTypeSize, int root)= 0;

    virtual void Reduce(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op,int root)= 0;
    virtual void AllReduce(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op)= 0;
    virtual void Scan(void* buffer, int count, void* recvBuffer, int dataTypeSize, OpType op)= 0;

    virtual void Abort() = 0;

};

}

}

#endif // ICOMMUNICATOR_H
