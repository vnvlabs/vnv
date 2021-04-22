//
// Created by ben on 4/22/21.
//

#ifndef VNV_MPICOMMUNICATOR_H
#define VNV_MPICOMMUNICATOR_H

#include <mpi.h>

namespace VnV {
namespace Communication {
namespace MPI {


class CommKeeper {
 public:
  MPI_Comm comm;
  long uniqueId = -1;
  CommKeeper(MPI_Comm comm_, long uid) {
    comm = comm_;
    uniqueId = uid;
  }
};

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
  virtual ~MPIRequest() {}
};

MPI_Comm castToMPIComm(ICommunicator_ptr ptr) {
  MPI_Comm mpicomm;
  if ( ptr->getName() == "serial") {
    mpicomm = MPI_COMM_SELF;
  } else if (ptr->getName() == "mpi") {
    VnV::Communication::MPI::CommKeeper* k = (VnV::Communication::MPI::CommKeeper*) ptr->getData();
    mpicomm = k->comm;
  } else {
    throw VnV::VnVExceptionBase("Cannot convert communicator to MPI");
  }
  return mpicomm;
}

}

}

}


#endif  // VNV_MPICOMMUNICATOR_H
