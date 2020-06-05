

#include "base/CommunicationStore.h"
#include <mpi.h>

#define MPICOMMNAME mpi;
#define MPICOMMNAME_S "mpi"


class MPIStatus : public IStatus {
public:
    MPI_Status status;
    int source() { return status.MPI_SOURCE;}
    int tag() { return status.MPI_TAG;}
    int error() { return status.MPI_ERROR;}
    int count() {
       int c;
       MPI_Get_count(&status, MPI_BYTE, &c);
       return c;
    }
    MPI_Status* stat() {
         return &status;
    }
};

class MPIReq : public IRequest {
public:
     MPI_Request req;
     IStatus wait() {
         MPIStatus s;
         MPI_Wait(request(), s.stat());
         return s;
     }
     MPI_Request* request() {
          return &req;
     }
};

IRequest_ptr create() {
     IRequest_ptr ptr;
     ptr.reset(new MPIReq());
     return ptr;
}

IStatus_ptr create() {
     IStatus_ptr ptr;
     ptr.reset(new MPIStatus());
     return ptr;
}

MPIStatus* cast(IStatus_ptr ptr) {
     if (MPIStatus *B = dynamic_cast<MPIStatus*>(ptr.get())) {
        return B;
     }
     throw VnV::VnVExceptionBase("Bad Cast");
}
MPIReq* cast(IRequest_ptr ptr) {
     if (MPIReq *B = dynamic_cast<MPIReq*>(ptr.get())) {
        return B;
     }
     throw VnV::VnVExceptionBase("Bad Cast");

}

void MPI_REDUCTION()


class MPICommunicator : public VnV::ICommunicator {

     MPI_Comm comm;
     MPI_Comm * comm_ptr() {
          return &comm;
     }

     static int sMap(int source) {
          return (source < 0) ? MPI_ANY_SOURCE : source;
     }
     static int tMap(int tag) {
          return (tag < 0 ) ? MPI_ANY_TAG : tag;
     }
     static std::map< DataType , int> dMap = {
          { DataType::Long , MPI_LONG },
          { DataType::Int , MPI_INT },
          { DataType::Double , MPI_DOUBLE },
          { DataType::Byte , MPI_BYTE },
          { DataType::Char , MPI_CHAR },
     };
     static std::map< int, DataType> idMap = {
          { MPI_LONG , DataType::Long },
          { MPI_INT, DataType::Int    },
          { MPI_DOUBLE, DataType::Double },
          { MPI_BYTE, DataType::Byte  },
          { MPI_CHAR , DataType::Char },
     };

     static MPI_Op getOp(IReduction_ptr ptr) {
          MPI_Op op;
          MPI_Op_create()
     }

     static std::shared_ptr<MPICommunicator> create() {
         auto s = std::make_shared<MPICommunicator>();
         return s;
     }
     static std::shared_ptr<MPICommunicator> createWorld() {
         auto s = create();
         s->comm = MPI_COMM_WORLD;
         return s;
     }
     static std::shared_ptr<MPICommunicator> createSelf() {
         auto s = create();
         s->comm = MPI_COMM_SELF;
         return s;
     }
     static std::shared_ptr<MPICommunicator> createDup(MPICommunicator* comm) {
          auto s = create();
          MPI_Comm_dup(comm->comm,s->comm_ptr());
          return s;
     }
     static std::shared_ptr<MPICommunicator> createRange(int start, int end, int stride, int tag, MPICommunicator *comm) {
          auto s = create();
          MPI_Group group,newGroup;
          MPI_Comm_group(comm->comm,&group);
          int range[1][3];
          range[1][0] = start; range[1][1] = end; range[1][2] = stride;
          MPI_Group_range_incl(group,1,range,&newGroup);
          MPI_Comm_create_group(comm->comm, newGroup, tag, s->comm_ptr());
          return s;
     }
     static std::shared_ptr<MPICommunicator> createList(std::vector<int>& ranks, int tag, MPICommunicator *comm) {
          auto s = create();
          MPI_Group group,newGroup;
          MPI_Comm_group(comm->comm,&group);
          MPI_Group_incl(group, ranks.size(), &ranks[0], &newGroup);
          MPI_Comm_create_group(comm->comm, newGroup, tag, s->comm_ptr());
          return s;
     }
     static std::shared_ptr<MPICommunicator> createSplit(int color, int key, MPICommunicator *comm) {
          auto s = create();
          MPI_Comm_split(comm->comm,color,key,s->comm_ptr());
          return s;
     }

     std::string name() {
          return MPICOMMNAME_S;
     }

     void init(int *argc, char **argv) override {
          MPI_Init(&argc,&argc);
     }
     void finalize() override {
          MPI_Finalize();
     }

     int size() {
          int s;
          MPI_Comm_size(comm,&s);
          return s;
     }
     int rank() {
          int s;
          MPI_Comm_rank(comm,&s);
          return s;
     }

     VnV::ICommunicator_ptr duplicate() {
          return createDup(this);
     }
     VnV::ICommunicator_ptr split() {
          return createSplit(color,key,this);
     }
     VnV::ICommunicator_ptr create(std::vector<int> &ranks, int tag) {
         return createList(ranks, tag, this);
     }
     VnV::ICommunicator_ptr create(int start, int end, int stride, int tag) {
          return createRange(start,end,stride,tag,this);
     }
     VnV::ICommunicator_ptr world() {
          return createWorld();
     }
     VnV::ICommunicator_ptr self() {
          return createSelf();
     }

     int abort(int errorCode){
          MPI_Abort(comm,errorCode);
     }
     std::pair<int,int> version() {
          int m,s;
          MPI_Get_version(&m,&s);
          return std::make_pair(m,s);
     }
     int initialized(){
          int i;
          MPI_Initialized(&i);
          return i;
     }
     std::string procName(){
          char name[MPI_MAX_PROCESSOR_NAME];
          int len;
          MPI_Get_processor_name(name,&len);
          return std::string(name,len);
     }
     double time() {
         return MPI_Wtime();
     }
     double tick(){
          return MPI_Wtick();
     }

     IStatus_ptr wait(IRequest_ptr &request) {
        return request->wait();
     }

     IStatus_vec wait_(IRequest_vec &requests, int &flag, bool any) {
          std::vector<MPI_Request> r(requests.size());
          std::vector<MPI_Status> s(requests.size());
          for (auto it : requests) {
            MPI_Status m;
            r.push_back(cast(it)->req);
            s.push_back(m);
          }
          if (!any) {
             MPI_Waitall(r.size(),&r[0],&s[0]);
          } else {
             MPI_Waitany(r.size(),&r[0],&flag, &s[0]);
          }
          std::vector<IStatus_ptr> stats;
          for (auto it : s) {
             IStatus_ptr ms = create();
             cast(ms)->status = s;
             stats.push_back(ms);
          }
          return stats;
     }

     IStatus_vec waitAll(IRequest_vec &request) {
          int x;
          return wait_(request, x, false);
     }
     std::pair<IStatus_vec,int> waitAny(IRequest_vec &request) {
          int x;
          auto s = wait_(request, x, true);
          return std::make_pair(std::move(s),x);
     }

    std::pair<IStatus_ptr,int> test(IRequest_ptr request) {
          IStatus_ptr s = create();
          int flag;
          MPI_Test(cast(request)->request(), &flag, cast(s)->stat());
          return std::make_pair(s,flag);
     }

     std::pair<IStatus_vec,int> test_(IRequest_vec &requests, int &flag, bool any) {
          std::vector<MPI_Request> r(requests.size());
          std::vector<MPI_Status> s(requests.size());
          for (auto it : requests) {
            MPI_Status m;
            r.push_back(cast(it)->req);
            s.push_back(m);
          }
          int res;
          if (!any) {
             MPI_Testall(r.size(),&r[0], &res, &s[0]);
          } else {
             MPI_Testany(r.size(),&r[0],&flag, &res, &s[0]);
          }
          std::vector<IStatus_ptr> stats;
          for (auto it : s) {
             IStatus_ptr ms = create();
             cast(ms)->status = s;
             stats.push_back(ms);
          }
          return std::make_pair(stats,res);
     }

     std::pair<IStatus_vec,int> testAll(IRequest_vec &request) {
          int x;
          return test_(request, x, false);
     }
     std::tuple<IStatus_vec,int,int> testAny(IRequest_vec &request) {
          int x;
          auto s = test_(request, x, true);
          return {s.first, s.second, x};
     }

     IStatus_ptr probe(int source, int tag) {
          IStatus_ptr s = create();
          MPI_Probe(sMap(source),tMap(tag),comm, cast(s)->stat());
          return s;
     }
     std::pair<IStatus_ptr,int> iProbe(int source, int tag) {
          IStatus_ptr s = create();
          int flag;
          MPI_Iprobe(sMap(source), tMap(tag), comm, flag, cast(s)->stat());
          return std::make_pair(s,flag);
     }
     void barrier() {
          MPI_Barrier(comm);
     }
     int count(IStatus_ptr status){
        return status->count();
     }

     void send(void* buffer, int size, int dest_, int tag_, DataType dtype, MessageType sync) {
          int dest = sMap(dest_);
          int tag = tMap(tag_);
          if (sync == MessageType::sync)
             MPI_Ssend(buffer,size,dMap[dtype],dest,tag,comm);
          else if (sync == MessageType::buffered) {
             MPI_Bsend(buffer,size,dMap[dtype],dest,tag,comm);
          } else {
             MPI_Send(buffer,size,dMap[dtype],dest,tag,comm);
          }
     }
     IRequest_ptr iSend(void* buffer, int size, int dest, int tag, DataType dtype, MessageType sync) {
          IRequest_ptr ptr = create();
          if (sync == MessageType::sync)
             MPI_Issend(buffer,size,dMap[dtype],sMap(dest),tMap(tag),comm, cast(ptr)->request());
          else if (sync == MessageType::buffered) {
             MPI_Ibsend(buffer,size,dMap[dtype],sMap(dest),tMap(tag),comm, cast(ptr)->request());
          } else {
             MPI_Isend(buffer,size,dMap[dtype],sMap(dest),tMap(tag),comm, cast(ptr)->request());
          }
          return ptr;
     }

     // No support for non blocking recieves -- cannot get the buffer size in
     // advance.
     std::tuple<IStatus_ptr,void*,int> recv(int source, int tag, DataType dtype) {
          IStatus_ptr ptr = probe(source,tag);
          int s = ptr->source();
          int t = ptr->tag();
          int size = ptr->count();
          int dsize;
          MPI_Type_size(dMap[dtype], &dsize);
          void* recvbuff = malloc( size*dsize );

          MPI_Recv(recvbuffer, size, dMap[dtype], s, t, comm, cast(ptr)->stat());
          return { ptr, recvbuff, size};
     }

     void* broadCast(void *data, int count, int root, DataType dtype) {
          if ( rank() != root ) {
             int dsize; MPI_Type_size(dMap[dtype],&dsize);
             data = malloc(dsize*count);
          }
          MPI_Bcast(data,count, dMap[dtype], root, comm);
          return data;
     }

     void* scatter(void* data, int count, int root, DataType dtype ) {
          int dsize; MPI_Type_size(dMap[dtype],&dsize);
          void* recvbuffer = malloc( count * dsize) ;
          MPI_Scatter(data, count, dMap[dtype],recvbuffer, count, dMap[type], root, comm);
          return recvbuffer;
     }

     void* gather(void* data, int count, int root, DataType dtype) {
          int dsize; MPI_Type_size(dMap[dtype],&dsize);
          void* recvbuffer = malloc( count * dsize) ;
          MPI_Gather(data,count,dMap[dtype],recvbuffer,count,dMap[dtype],root, comm);
     }

     void* reduce(void* data, int count, DataType dtype, IReduction_ptr op, int root) {

          void* recvdata = nullptr;
          if (rank() == root) {
                int size = size();
                int dsize; MPI_Type_size(dMap[dtype], &dsize);
                recvdata = malloc( dsize * count * size );
          }
          MPI_Reduce(data, recvdata,count, dMap[dtype], oMap(op), root, comm);
          return recvdata;
     }





































































};
