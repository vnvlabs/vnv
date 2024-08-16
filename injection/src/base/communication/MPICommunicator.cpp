

#include <mpi.h>
#include <unistd.h>

#include <iostream>
#include <map>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"
#include "base/communication/ICommunicator.h"

// CommType conflicts with CommType in openmpi.
using VnVCommType = VnV::VnVCommType;

namespace VnV
{
  namespace Communication
  {
    
    namespace MPI
    {

      struct CommData {
          MPI_Comm comm;
      };



      class MPIStatus : public IStatus
      {
      public:
        MPI_Status status;
        int source() { return status.MPI_SOURCE; }
        int tag() { return status.MPI_TAG; }
        int error() { return status.MPI_ERROR; }
        virtual ~MPIStatus() {}
      };

      class MPIRequest : public IRequest
      {
      public:
        MPI_Request request;
        virtual ~MPIRequest() {}
      };

      class MPICommunicator : public ICommunicator
      {

        static int destMap(int dest) { return (dest < 0) ? MPI_ANY_SOURCE : dest; }
        static int tagMap(int tag) { return (tag < 0) ? MPI_ANY_TAG : tag; }
        static MPIRequest *cast(IRequest_ptr ptr) { return (MPIRequest *)ptr.get(); }
        static MPIStatus *cast(IStatus_ptr ptr) { return (MPIStatus *)ptr.get(); }

        static bool finalizeMPI;

        static MPI_Datatype &getDataType(long size)
        {
          static std::map<long, MPI_Datatype> dataTypes = {{sizeof(double), MPI_DOUBLE},
                                                           {sizeof(int), MPI_INT},
                                                           {sizeof(float), MPI_FLOAT},
                                                           {sizeof(char), MPI_CHAR},
                                                           {sizeof(wchar_t), MPI_WCHAR},
                                                           {sizeof(bool), MPI_C_BOOL},
                                                           {sizeof(short), MPI_SHORT},
                                                           {sizeof(long long), MPI_LONG_LONG},
                                                           {sizeof(long double), MPI_LONG_DOUBLE}};

          auto it = dataTypes.find(size);
          if (it != dataTypes.end())
          {
            return it->second;
          }

          MPI_Datatype dataType;
          dataTypes.insert(std::make_pair(size, dataType));
          MPI_Type_contiguous(size, MPI_BYTE, &(dataTypes[size]));
          MPI_Type_commit(&(dataTypes[size]));
          return dataTypes[size];
        }

        void Initialize_()
        {
          int flag;
          MPI_Initialized(&flag);
          if (!flag)
          {

            MPI_Init(NULL, NULL);
            VnV_Warn(VNVPACKAGENAME,
                     "We are Initializing MPI for you. If this is not the intended "
                     "behaviour, please "
                     "call MPI_Init before called INJECTION_INITIALIZE. ");
            finalizeMPI = true;
          }
        }

        void Initialize() override
        {
          Initialize_();
        }

        void Finalize() override
        {
          int flag;
          MPI_Finalized(&flag);
          if (finalizeMPI && !flag)
          {
            MPI_Finalize();
          }
        }

        static void custom_reduction_function(void *in, void *inout, int *len, MPI_Datatype *data)
        {
          OpTypeEncodedReduction(in, inout, len);
        }

        static MPI_Op getEncodedNonCommuteOp()
        {
          static MPI_Op nonCommuteOperation;
          static bool nonCommuteInitialized;
          if (!nonCommuteInitialized)
          {
            MPI_Op_create(&custom_reduction_function, false, &nonCommuteOperation);
            nonCommuteInitialized = true;
          }
          return nonCommuteOperation;
        }

        static MPI_Op getEncodedCommuteOp()
        {
          static MPI_Op commuteOperation;
          static bool commuteInitialized;
          if (!commuteInitialized)
          {
            MPI_Op_create(&custom_reduction_function, true, &commuteOperation);
            commuteInitialized = true;
          }
          return commuteOperation;
        }

        int Size_()
        {
          int x;
          MPI_Comm_size(comm, &x);
          return x;
        }

        static MPI_Op getOp(OpType type)
        {
          switch (type)
          {
          case OpType::SUM:
            return MPI_SUM;
          case OpType::PROD:
            return MPI_PROD;
          case OpType::MIN:
            return MPI_MIN;
          case OpType::MAX:
            return MPI_MAX;
          case OpType::LOR:
            return MPI_LOR;
          case OpType::LAND:
            return MPI_LAND;
          case OpType::ENCODED_COMMUTE:
            return getEncodedCommuteOp();
          case OpType::ENCODED_NONCOMMUTE:
            return getEncodedNonCommuteOp();
          }
          throw INJECTION_BUG_REPORT_("Un supported Operation type");
        }

        MPI_Comm comm;

        // ICommunicator interface
      public:
        VnVCommType mtype;
        long commUniqueId = -1;
        int worldSize, worldRank;



        MPICommunicator(VnVCommType type) : mtype(type)
        {
          Initialize_();
          MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
          MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

          if (type == VnVCommType::Self)
          {
            set(MPI_COMM_SELF);
          }
          else if (type == VnVCommType::World)
          {
            set(MPI_COMM_WORLD);
          }
        }

        MPICommunicator(MPI_Comm comm ) : mtype(VnVCommType::Default) {
           Initialize_();
           MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
           MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
           set(comm);
        }

 
        CommData& getCommData() {
        
           static std::map<long, CommData> store;
           auto uid = uniqueId();
           auto it = store.find(uid);
           if (it == store.end()) {
              store[uid] = {comm};
              return store[uid];
           }
           return it->second;
        
        }


        VnV_Comm get() override
        {
          VnV_Comm vnv_comm;
          vnv_comm.data = (void*) &getCommData();
          return vnv_comm;
        }

        ICommunicator_ptr custom(VnV_Comm comm) override
        {
          MPICommunicator *p = new MPICommunicator(VnVCommType::Default);
          p->set(comm);
          return ICommunicator_ptr(p);
        }

        void set(VnV_Comm comm) {
            CommData* d = (CommData*) comm.data;
            set(d->comm);
        }

        void set(MPI_Comm comm_) 
        {
         
          comm = comm_;
          if (commUniqueId < 0)
          {
            if (Size_() == worldSize)
            {
              commUniqueId = worldSize == 1 ? getpid() : worldSize;
              return;
            }
            else if (Size_() == 1)
            {
              commUniqueId = getpid();
              return;
            }
            else
            {

              static std::vector<std::pair<MPI_Comm, long>> cached_comms;
              static int crank = 0;

              for (auto &it : cached_comms)
              {
                MPI_Comm_compare(comm, it.first, &crank);
                if (crank != MPI_UNEQUAL)
                {
                  commUniqueId = it.second;
                  return;
                }
              }

              MPI_Group grp, world_grp;
              MPI_Comm_group(MPI_COMM_WORLD, &world_grp);
              MPI_Comm_group(comm, &grp);

              int grp_size = Size_();

              std::vector<int> ranks(grp_size);
              std::iota(ranks.begin(), ranks.end(), 0);

              std::vector<int> wranks(grp_size);
              MPI_Group_translate_ranks(grp, grp_size, &(ranks[0]), world_grp, &(wranks[0]));
              MPI_Group_free(&grp);
              MPI_Group_free(&world_grp);

              commUniqueId = VnV::HashUtils::vectorHash(wranks);
              cached_comms.push_back({comm, commUniqueId});
            }
          }
        }

        bool contains(long proc) override
        {
          int wrank;
          MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
          int me = (wrank == proc) ? 1 : 0;
          MPI_Allreduce(&me, &me, 1, MPI_INT, MPI_MAX, comm);
          return me == 1;
        }

     
        long uniqueId() override { return commUniqueId; }

        int Size() override
        {
          return Size_();
        }

        int Rank() override
        {
          int x;
          MPI_Comm_rank(comm, &x);
          return x;
        }

        void Barrier() override { MPI_Barrier(comm); }

        std::string getPackage() override { return "VNV"; }
        std::string getName() override { return "mpi"; }

        std::string ProcessorName() override
        {
          int size;
          char name[MPI_MAX_PROCESSOR_NAME];
          MPI_Get_processor_name(name, &size);
          std::string s(name);
          return s;
        }

        double time() override { return MPI_Wtime(); }

        double tick() override { return MPI_Wtick(); }

        ICommunicator_ptr duplicate() override
        {
          MPICommunicator *p = new MPICommunicator(VnVCommType::Default);

          MPI_Comm newComm;
          MPI_Comm_dup(comm, &newComm);
          p->set(newComm);
          return ICommunicator_ptr(p);
        }

        ICommunicator_ptr split(int color, int key) override
        {
          MPICommunicator *p = new MPICommunicator(VnVCommType::Default);

          MPI_Comm newComm;
          MPI_Comm_split(comm, color, key, &newComm);
          p->set(newComm);
          return ICommunicator_ptr(p);
        }

        ICommunicator_ptr create(std::vector<int> &ranks, int tag) override
        {
          MPICommunicator *p = new MPICommunicator(VnVCommType::Default);
          MPI_Group group, newGroup;
          MPI_Comm_group(comm, &group);
          MPI_Group_incl(group, ranks.size(), &ranks[0], &newGroup);

          MPI_Comm newComm;
          MPI_Comm_create_group(comm, newGroup, tag, &newComm);
          p->set(newComm);

          return ICommunicator_ptr(p);
        }

        virtual int VersionMajor() override
        {
          int major, minor;
          MPI_Get_version(&major, &minor);
          return major;
        }

        virtual int VersionMinor() override
        {
          int major, minor;
          MPI_Get_version(&major, &minor);
          return minor;
        }
        
        virtual std::string VersionLibrary() override
        {
          int resultlen;
          char version[MPI_MAX_LIBRARY_VERSION_STRING];
          MPI_Get_library_version(version, &resultlen);
          return std::string(version, resultlen);
        }

        ICommunicator_ptr world() override { return ICommunicator_ptr(new MPICommunicator(VnVCommType::World)); }

   
        ICommunicator_ptr self() override { return ICommunicator_ptr(new MPICommunicator(VnVCommType::Self)); }

        // Want procs in range [start,end). Make sure to put end-1.
        ICommunicator_ptr create(int start, int end, int stride, int tag) override
        {
          MPICommunicator *p = new MPICommunicator(VnVCommType::Default);

          MPI_Group group, newGroup;
          MPI_Comm_group(comm, &group);

          int range[1][3];

          range[0][0] = start;
          range[0][1] = end - 1;
          range[0][2] = stride;
          MPI_Group_range_incl(group, 1, range, &newGroup);

          MPI_Comm newComm;
          MPI_Comm_create_group(comm, newGroup, tagMap(tag), &newComm);
          p->set(newComm);

          return ICommunicator_ptr(p);
        }

        /*CommCompareType compare(ICommunicator_ptr ptr) override {
          int flag;
          MPI_Comm* other = (MPI_Comm*)ptr->getData();
          MPI_Comm_compare(comm, *other, &flag);
          switch (flag) {
          case MPI_IDENT:
            return CommCompareType::EXACT;
          case MPI_CONGRUENT:
            return CommCompareType::GROUP;
          case MPI_SIMILAR:
            return CommCompareType::SIMILAR;
          case MPI_UNEQUAL:
            return CommCompareType::UNEQUAL;
          }
          return CommCompareType::UNEQUAL;
        }*/

        // Return true if communicator is entirly contained in this communicator.
        // bool contains(ICommunicator_ptr ptr) override {
        //  // If that comm is contained in this comm, then the intersection
        //  // between that comm and this comm with be equal to that comm;
        //  MPI_Group me, them, intersect;
        //  MPI_Comm* other = (MPI_Comm*)ptr->getData();
        // MPI_Comm_group(comm, &me);
        //  MPI_Comm_group(*other, &them);
        //  MPI_Group_intersection(me, them, &intersect);
        //  int contained;
        //  MPI_Group_compare(them, intersect, &contained);
        //  return contained != MPI_UNEQUAL;
        // }

        void Send(void *buffer, int count, int dest, int tag, int dataTypeSize) override
        {
          MPI_Send(buffer, count, getDataType(dataTypeSize), destMap(dest), tagMap(tag), comm);
        }

        IRequest_ptr ISend(void *buffer, int count, int dest, int tag, int dataTypeSize) override
        {
          MPIRequest *request = new MPIRequest();
          MPI_Isend(buffer, count, getDataType(dataTypeSize), destMap(dest), tagMap(tag), comm, &(request->request));
          IRequest_ptr p(request);
          return p;
        }

        IStatus_ptr Recv(void *buffer, int count, int dest, int tag, int dataTypeSize) override
        {
          MPIStatus *stat = new MPIStatus();
          MPI_Recv(buffer, count, getDataType(dataTypeSize), destMap(dest), tagMap(tag), comm, &(stat->status));
          IStatus_ptr p(stat);
          return p;
        }

        IRequest_ptr IRecv(void *buffer, int count, int source, int tag, int dataTypeSize) override
        {
          MPIRequest *request = new MPIRequest();
          MPI_Irecv(buffer, count, getDataType(dataTypeSize), destMap(source), tagMap(tag), comm, &(request->request));
          IRequest_ptr p(request);
          return p;
        }

        IStatus_ptr Wait(IRequest_ptr ptr) override
        {
          MPIStatus *stat = new MPIStatus();
          MPI_Wait(&(cast(ptr)->request), &(stat->status));
          IStatus_ptr p(stat);
          return p;
        }

        IStatus_vec WaitAll(IRequest_vec &vec) override
        {
          IStatus_vec results(vec.size());
          std::vector<MPI_Request> requests(vec.size());
          std::vector<MPI_Status> statuses(vec.size());
          for (auto it : vec)
          {
            requests.push_back(cast(it)->request);
            statuses.push_back(MPI_Status());
          }
          MPI_Waitall(vec.size(), &(requests[0]), &(statuses[0]));
          for (auto it : statuses)
          {
            MPIStatus *s = new MPIStatus();
            s->status = it;
            IStatus_ptr p(s);
            results.push_back(p);
          }
          return results;
        }

        std::pair<IStatus_ptr, int> WaitAny(IRequest_vec &vec) override
        {
          std::vector<MPI_Request> requests(vec.size());
          std::vector<MPI_Status> statuses(vec.size());
          for (auto it : vec)
          {
            requests.push_back(cast(it)->request);
            statuses.push_back(MPI_Status());
          }
          MPIStatus *s = new MPIStatus();
          int index = 0;
          MPI_Waitany(vec.size(), &(requests[0]), &index, &(s->status));
          IStatus_ptr p(s);
          return std::make_pair(p, index);
        }

        int Count(IStatus_ptr status, int dataTypeSize) override
        {
          MPIStatus *stat = cast(status);
          int count;
          MPI_Get_count(&(stat->status), getDataType(dataTypeSize), &count);
          return count;
        }

        IStatus_ptr Probe(int source, int tag) override
        {
          MPIStatus *stat = new MPIStatus();
          MPI_Probe(destMap(source), tagMap(tag), comm, &(stat->status));
          IStatus_ptr s(stat);
          return s;
        }

        std::pair<IStatus_ptr, int> IProbe(int source, int tag) override
        {
          MPIStatus *stat = new MPIStatus();
          int flag;
          MPI_Iprobe(destMap(source), tagMap(tag), comm, &flag, &(stat->status));
          IStatus_ptr s(stat);
          return std::make_pair(s, flag);
        }

        std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr) override
        {
          MPIStatus *stat = new MPIStatus();
          int flag;
          MPI_Test(&(cast(ptr)->request), &flag, &(stat->status));
          IStatus_ptr s(stat);
          return std::make_pair(s, flag);
        }

        std::pair<IStatus_vec, int> TestAll(IRequest_vec &vec) override
        {
          std::vector<MPI_Request> requests(vec.size());
          std::vector<MPI_Status> statuses(vec.size());
          for (auto it : vec)
          {
            requests.push_back(cast(it)->request);
            statuses.push_back(MPI_Status());
          };
          int flag;
          MPI_Testall(vec.size(), &(requests[0]), &flag, &(statuses[0]));
          IStatus_vec results;
          for (auto it : statuses)
          {
            MPIStatus *s = new MPIStatus();
            s->status = it;
            results.push_back(IStatus_ptr(s));
          }
          return std::make_pair(results, flag);
        }

        std::tuple<IStatus_ptr, int, int> TestAny(IRequest_vec &vec) override
        {
          std::vector<MPI_Request> requests(vec.size());
          std::vector<MPI_Status> statuses(vec.size());
          std::vector<std::pair<IStatus_ptr, int>> results;
          for (auto it : vec)
          {
            requests.push_back(cast(it)->request);
            statuses.push_back(MPI_Status());
          };
          int flag;
          MPIStatus *stat = new MPIStatus();
          int index;
          MPI_Testany(vec.size(), &(requests[0]), &index, &flag, &(stat->status));
          IStatus_ptr p(stat);
          return {p, index, flag};
        }

        void Gather(void *buffer, int count, void *recvBuffer, int dataTypeSize, int root) override
        {
          MPI_Gather(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize), root, comm);
        }

        void AllGather(void *buffer, int count, void *recvBuffer, int dataTypeSize) override
        {
          MPI_Allgather(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize), comm);
        }
        void GatherV(void *buffer, int count, void *recvBuffer, int *recvCount, int *recvDispl, int dataTypeSize, int root) override
        {
          MPI_Gatherv(buffer, count, getDataType(dataTypeSize), recvBuffer, recvCount, recvDispl, getDataType(dataTypeSize),
                      root, comm);
        }

        void AllGatherV(void *buffer, int count, void *recvBuffer, int *recvCount, int *recvDispl, int dataTypeSize) override
        {
          MPI_Allgatherv(buffer, count, getDataType(dataTypeSize), recvBuffer, recvCount, recvDispl,
                         getDataType(dataTypeSize), comm);
        }

        void BroadCast(void *buffer, int count, int dataTypeSize, int root) override
        {
          MPI_Bcast(buffer, count, getDataType(dataTypeSize), root, comm);
        }

        void AllToAll(void *buffer, int count, void *recvBuffer, int dataTypeSize) override
        {
          MPI_Alltoall(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize), comm);
        }

        void Scatter(void *buffer, int count, void *recvBuffer, int dataTypeSize, int root) override
        {
          MPI_Scatter(buffer, count, getDataType(dataTypeSize), recvBuffer, count, getDataType(dataTypeSize), root, comm);
        }

        void Reduce(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op, int root) override
        {
          MPI_Reduce(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), root, comm);

          char *coutvec = (char *)recvBuffer;
        }

        void AllReduce(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op) override
        {
          MPI_Allreduce(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), comm);
        }

        void Scan(void *buffer, int count, void *recvBuffer, int dataTypeSize, OpType op) override
        {
          MPI_Scan(buffer, recvBuffer, count, getDataType(dataTypeSize), getOp(op), comm);
        }

        void Abort(int errorcode) override { MPI_Abort(comm, errorcode); }
      };

      bool MPICommunicator::finalizeMPI = false;

      
      VnV_Comm from_mpi_comm(MPI_Comm comm) {
         MPICommunicator c(comm);
         return c.get();
      }

    } // namespace MPI
  } // namespace Communication
} // namespace VnV

namespace VnV {                                         \
  namespace Communication {                               \

    VnV::ICommunicator* create_mpi_communicator(VnV::VnVCommType type)  {
      return new VnV::Communication::MPI::MPICommunicator(type); 
    }

    VnV::ICommunicator* create_mpi_communicator(MPI_Comm comm)  {
      return new VnV::Communication::MPI::MPICommunicator(comm); 
    }

  }                                                       \
}                                                       \

