

#include "interfaces/ICommunicator.h"
#include <mpi.h>
#include <map>

#include "base/exceptions.h"

using namespace VnV::Communication;

class MPIStatus : public IStatus {
public:
  MPI_Status status;
  int source() { return status.MPI_SOURCE; }
  int tag() { return status.MPI_TAG; }
  int error() { return status.MPI_ERROR; }
  virtual ~MPIStatus() {}
};

class MPIRequest : public IRequest {
public:
  MPI_Request request;
  virtual ~MPIRequest(){}
};


class MPICommunicator : public ICommunicator {

  static int destMap(int dest) {
     return (dest < 0 ) ? MPI_ANY_SOURCE : dest;
  }
  static int tagMap(int tag) {
    return (tag<0) ? MPI_ANY_TAG : tag;
  }
  static MPIRequest* cast(IRequest_ptr ptr) {
     return (MPIRequest*) ptr.get();
  }
  static MPIStatus* cast(IStatus_ptr ptr) {
     return (MPIStatus*) ptr.get();
  }



  static int getDataType(long size) {

    static std::map<long, int> dataTypes = {
       { sizeof(double) ,      MPI_DOUBLE },
       { sizeof(int),          MPI_INT },
       { sizeof(float),        MPI_FLOAT },
       { sizeof(char),         MPI_CHAR },
       { sizeof(wchar_t),      MPI_WCHAR },
       { sizeof(bool),         MPI_C_BOOL },
       { sizeof(short),        MPI_SHORT },
       { sizeof(long long) ,   MPI_LONG_LONG },
       { sizeof(long double),  MPI_LONG_DOUBLE}
    };

    auto it = dataTypes.find(size);
    if (it != dataTypes.end()) {
       return it->second;
    }

    MPI_Datatype dataType;
    MPI_Type_contiguous(size, MPI_BYTE, &dataType);
    MPI_Type_commit(&dataType);
    dataTypes[size] = dataType;
    return dataType;
  }

  static void custom_reduction_function(void* in, void* inout, int* len, MPI_Datatype *data) {
     OpTypeEncodedReduction(in, inout, len);
  }

  static MPI_Op getEncodedNonCommuteOp() {
     static MPI_Op nonCommuteOperation;
     static bool nonCommuteInitialized;
     if (!nonCommuteInitialized) {
        MPI_Op_create(&custom_reduction_function, false, &nonCommuteOperation);
        nonCommuteInitialized = true;
     }
     return nonCommuteOperation;
  }

  static MPI_Op getEncodedCommuteOp() {
     static MPI_Op commuteOperation;
     static bool commuteInitialized;
     if (!commuteInitialized) {
        MPI_Op_create(&custom_reduction_function, true, &commuteOperation);
        commuteInitialized = true;
     }
     return commuteOperation;
  }


  static MPI_Op getOp(OpType type) {
    switch (type) {
      case OpType::SUM: return MPI_SUM;
      case OpType::PROD: return MPI_PROD;
      case OpType::MIN: return MPI_MIN;
      case OpType::MAX: return MPI_MAX;
      case OpType::LOR: return MPI_LOR;
      case OpType::LAND: return MPI_LAND;
      case OpType::ENCODED_COMMUTE:
        return getEncodedCommuteOp();
      case OpType::ENCODED_NONCOMMUTE:
        return getEncodedNonCommuteOp();
    }
    throw VnV::VnVExceptionBase("Un supported Operation type");
  }


  // ICommunicator interface
public:
  MPI_Comm comm;
  CommType mtype;

  MPICommunicator(CommType type) : mtype(type) {
    if (type == CommType::Self) {
       comm = MPI_COMM_SELF;
    } else if ( type == CommType::World) {
       comm = MPI_COMM_WORLD;
    }
  }

  int setData(void* data) override {
    comm = *((MPI_Comm*) data);
  }
  void* getData() {
    return &comm;
  }

  int uniqueId() {
     return comm;
  }

  int Size() { int x; MPI_Comm_size(comm,&x); return x;}

  int Rank() { int x; MPI_Comm_rank(comm,&x); return x;}

  void Barrier() { MPI_Barrier(comm);}

  std::string ProcessorName() {
    int size;
    char name[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(name,&size);
    std::string s(name);
    return s;
  }
  double time() { return MPI_Wtime(); }

  double tick() { return MPI_Wtick(); }

  ICommunicator_ptr duplicate() {
     MPICommunicator *p = new MPICommunicator(CommType::Default);
     MPI_Comm_dup(comm, &(p->comm));
     return ICommunicator_ptr(p);
  }

  ICommunicator_ptr split(int color, int key) {
    MPICommunicator *p = new MPICommunicator(CommType::Default);
    MPI_Comm_split(comm, color, key, &(p->comm));
    return ICommunicator_ptr(p);
  }

  ICommunicator_ptr create(std::vector<int> &ranks, int tag) {
    MPICommunicator *p = new MPICommunicator(CommType::Default);
    MPI_Group group, newGroup;
    MPI_Comm_group(comm, &group);
    MPI_Group_incl(group, ranks.size(), &ranks[0], &newGroup);
    MPI_Comm_create_group(comm, newGroup, tag, &(p->comm));
    return ICommunicator_ptr(p);
  }

  // Want procs in range [start,end). Make sure to put end-1.
  ICommunicator_ptr create(int start, int end, int stride, int tag) {
    MPICommunicator *p =  new MPICommunicator(CommType::Default);
    MPI_Group group, newGroup;
    MPI_Comm_group(comm, &group);
    int range[1][3];
    range[0][0] = start;
    range[0][1] = end-1;
    range[0][2] = stride;
    MPI_Group_range_incl(group, 1, range, &newGroup);
    MPI_Comm_create_group(comm, newGroup, tagMap(tag), &(p->comm));
    return ICommunicator_ptr(p);
  }

  CommCompareType compare(ICommunicator_ptr ptr){
    int flag;
    MPI_Comm *other = (MPI_Comm*) ptr->getData();
    MPI_Comm_compare(comm,*other, &flag);
    switch (flag) {
      case MPI_IDENT: return CommCompareType::EXACT;
      case MPI_CONGRUENT: return CommCompareType::GROUP;
      case MPI_SIMILAR: return CommCompareType::SIMILAR;
      case MPI_UNEQUAL: return CommCompareType::UNEQUAL;
    }
    return CommCompareType::UNEQUAL;
  }

  // Return true if communicator is entirly contained in this communicator.
  bool contains(ICommunicator_ptr ptr) {
     // If that comm is contained in this comm, then the intersection
     // between that comm and this comm with be equal to that comm;
     MPI_Group me, them, intersect;
     MPI_Comm *other = (MPI_Comm*) ptr->getData();
     MPI_Comm_group(comm,&me);
     MPI_Comm_group(*other,&them);
     MPI_Group_intersection(me,them,&intersect);
     int contained;
     MPI_Group_compare(them,intersect,&contained);
     return contained != MPI_UNEQUAL;
  }

  void Send(void *buffer, int count, int dest, int tag, int dataTypeSize) {
     MPI_Send(buffer, count, getDataType(dataTypeSize), destMap(dest), tagMap(tag), comm);
  }

  IRequest_ptr ISend(void *buffer, int count, int dest, int tag, int dataTypeSize) {
    MPIRequest *request = new MPIRequest();
    MPI_Isend(buffer, count, getDataType(dataTypeSize),destMap(dest),tagMap(tag), comm, &(request->request));
    IRequest_ptr p(request);
    return p;
  }

  IStatus_ptr Recv(void *buffer, int count, int dest, int tag, int dataTypeSize) {
    MPIStatus *stat = new MPIStatus();
    MPI_Recv(buffer, count, getDataType(dataTypeSize), destMap(dest),tagMap(tag),comm, &(stat->status));
    IStatus_ptr p(stat);
    return p;
  }

  IRequest_ptr IRecv(void *buffer, int count, int source, int tag, int dataTypeSize) {
    MPIRequest *request = new MPIRequest();
    MPI_Irecv(buffer, count, getDataType(dataTypeSize),destMap(source), tagMap(tag), comm, &(request->request));
    IRequest_ptr p(request);
    return p;
  }

  IStatus_ptr Wait(IRequest_ptr ptr) {
    MPIStatus *stat = new MPIStatus();
    MPI_Wait(&(cast(ptr)->request), &(stat->status));
    IStatus_ptr p(stat);
    return p;
  }

  IStatus_vec WaitAll(IRequest_vec &vec) {
     IStatus_vec results(vec.size());
     std::vector<MPI_Request> requests(vec.size());
     std::vector<MPI_Status> statuses(vec.size());
     for (auto it : vec) {
       requests.push_back(cast(it)->request);
       statuses.push_back(MPI_Status());
     }
     MPI_Waitall(vec.size(),&(requests[0]), &(statuses[0]));
     for (auto it : statuses) {
         MPIStatus *s = new MPIStatus();
         s->status = it;
         IStatus_ptr p(s);
         results.push_back(p);
     }
     return results;
  }

  std::pair<IStatus_ptr, int> WaitAny(IRequest_vec &vec) {
     std::vector<MPI_Request> requests(vec.size());
     std::vector<MPI_Status> statuses(vec.size());
     for (auto it : vec) {
       requests.push_back(cast(it)->request);
       statuses.push_back(MPI_Status());
     }
     MPIStatus *s = new MPIStatus();
     int index = 0;
     MPI_Waitany(vec.size(),&(requests[0]), &index, &(s->status) );
     IStatus_ptr p(s);
     return std::make_pair(p,index);
  }

  int Count(IStatus_ptr status, int dataTypeSize) {
      MPIStatus* stat = cast(status);
      int count;
      MPI_Get_count(&(stat->status), getDataType(dataTypeSize), &count);
      return count;
  }

  IStatus_ptr Probe(int source, int tag) {
     MPIStatus *stat = new MPIStatus();
     MPI_Probe(destMap(source),tagMap(tag), comm, &(stat->status));
     IStatus_ptr s(stat);
     return s;
  }

  std::pair<IStatus_ptr, int> IProbe(int source, int tag) {
     MPIStatus *stat = new MPIStatus();
     int flag;
     MPI_Iprobe(destMap(source),tagMap(tag), comm, &flag, &(stat->status));
     IStatus_ptr s(stat);
     return std::make_pair(s,flag);
  }

  std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr) {
      MPIStatus * stat = new MPIStatus();
      int flag;
      MPI_Test(&(cast(ptr)->request), &flag, &(stat->status));
      IStatus_ptr s(stat);
      return std::make_pair(s,flag);
  }

  std::pair<IStatus_vec, int> TestAll(IRequest_vec &vec) {
     std::vector<MPI_Request> requests(vec.size());
     std::vector<MPI_Status> statuses(vec.size());
     for ( auto it : vec ) {
       requests.push_back(cast(it)->request);
       statuses.push_back(MPI_Status());
     };
     int flag;
     MPI_Testall(vec.size(),&(requests[0]), &flag, &(statuses[0]));
     IStatus_vec results;
     for (auto it : statuses ) {
        MPIStatus *s = new MPIStatus();
        s->status = it;
        results.push_back(IStatus_ptr(s));
     }
     return std::make_pair(results,flag);
  }

  std::tuple<IStatus_ptr, int, int> TestAny(IRequest_vec &vec) {
     std::vector<MPI_Request> requests(vec.size());
     std::vector<MPI_Status> statuses(vec.size());
     std::vector<std::pair<IStatus_ptr,int>> results;
     for ( auto it : vec ) {
       requests.push_back(cast(it)->request);
       statuses.push_back(MPI_Status());
     };
     int flag;
     MPIStatus *stat = new MPIStatus();
     int index;
     MPI_Testany(vec.size(),&(requests[0]), &index, &flag, &(stat->status));
     IStatus_ptr p(stat);
     return {p,index,flag};
  }

  void Gather(void *buffer, int count, void *recvBuffer, int dataTypeSize, int root) {
     MPI_Gather(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize),root,comm);
  }

  void AllGather(void *buffer, int count, void *recvBuffer, int dataTypeSize) {
     MPI_Allgather(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize),comm);
  }

  void BroadCast(void *buffer, int count, int dataTypeSize, int root) {
    MPI_Bcast(buffer, count, getDataType(dataTypeSize), root, comm);
  }

  void AllToAll(void *buffer, int count, void *recvBuffer, int dataTypeSize) {
     MPI_Alltoall(buffer,count,getDataType(dataTypeSize), recvBuffer,count, getDataType(dataTypeSize),comm);
  }

  void Scatter(void *buffer, int count, void *recvBuffer, int dataTypeSize, int root) {
     MPI_Scatter(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize), root, comm);
  }

  void Reduce(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op, int root) {
     MPI_Reduce(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), root, comm );
  }

  void AllReduce(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op) {
     MPI_Allreduce(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), comm );
  }
  void Scan(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op) {
     MPI_Scan(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), comm );
  }
  void Abort(int errorcode) {
    MPI_Abort(comm, errorcode);
  }

};
INJECTION_COMM(mpi) {
  return new MPICommunicator(type);
}
