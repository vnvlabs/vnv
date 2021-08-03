#include "base/Communication.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/DataTypeStore.h"
#include "base/exceptions.h"


#include <algorithm>

using namespace VnV;

// TODO: Switch out all the malloc and free statements for std::vector<char>
// objects

IStatus_ptr DataTypeCommunication::Probe(int source, int tag) {
  return comm->Probe(source, tag);
}

std::pair<IStatus_ptr, int> DataTypeCommunication::IProbe(int source, int tag) {
  return comm->IProbe(source, tag);
}

IStatus_ptr DataTypeCommunication::Wait(IRequest_ptr ptr) {
  auto s = comm->Wait(ptr);
  ptr->ready = true;
  return s;
}

IStatus_vec DataTypeCommunication::WaitAll(IRequest_vec& vec) {
  auto s = comm->WaitAll(vec);
  for (auto it : vec) it->ready = true;
  return s;
}

std::pair<IStatus_ptr, int> DataTypeCommunication::WaitAny(IRequest_vec& vec) {
  auto s = comm->WaitAny(vec);
  vec[s.second]->ready = true;
  return s;
}

std::pair<IStatus_ptr, int> DataTypeCommunication::Test(IRequest_ptr ptr) {
  auto s = comm->Test(ptr);
  if (s.second) {
    ptr->ready = true;
  }
  return s;
}

std::pair<IStatus_vec, int> DataTypeCommunication::TestAll(IRequest_vec& vec) {
  auto s = comm->TestAll(vec);
  if (s.second) {
    for (std::size_t i = 0; i < s.first.size(); i++) {
      vec[i]->ready = true;
    }
  }
  return s;
}

std::tuple<IStatus_ptr, int, int> DataTypeCommunication::TestAny(
    IRequest_vec& vec) {
  auto s = comm->TestAny(vec);
  if (std::get<2>(s)) {
    vec[std::get<1>(s)]->ready = true;
  }
  return s;
}

IRequest_ptr DataTypeCommunication::Send(IDataType_vec& data, int dest, int tag,
                                         bool blocking) {
  long dataSize = data[0]->maxSize() + sizeof(long long);
  char* buffer = (char*)malloc(dataSize * data.size());
  long long dataKey = data[0]->getKey();
  for (int i = 0; i < data.size(); i++) {
    long long* buff = (long long*)&(buffer[i * dataSize]);
    buff[0] = dataKey;
    data[i]->pack(&(buff[1]));
  }
  if (blocking) {
    comm->Send(buffer, data.size(), dest, tag, dataSize);
    return nullptr;
  } else {
    IRequest_ptr ptr = comm->ISend(buffer, data.size(), dest, tag, dataSize);
    ptr->buffer = buffer;  // set the buffer so we can free on destruction.
    return ptr;
  }
}

std::pair<IDataType_vec, IStatus_ptr> DataTypeCommunication::Recv(
    int count, long long dataType, int dest, int tag) {
  IDataType_ptr ptr = DataTypeStore::instance().getDataType(dataType);
  long dataSize = ptr->maxSize() + sizeof(long long);
  char* buffer = (char*)malloc(count * dataSize);
  IStatus_ptr status = comm->Recv(buffer, count, dest, tag, dataSize);

  // Unwrap.
  IDataType_vec results;
  for (int i = 0; i < count; i++) {
    IDataType_ptr ptr = DataTypeStore::instance().getDataType(dataType);
    long long* buff = (long long*)&(buffer[i * dataSize]);
    ptr->unpack(&(buff[1]));
    results.push_back(ptr);
  }
  free(buffer);
  return std::make_pair(results, status);
}

IRequest_ptr DataTypeCommunication::IRecv(int count, long long dataType,
                                          int dest, int tag) {
  IDataType_ptr dptr = DataTypeStore::instance().getDataType(dataType);
  long dataSize = dptr->maxSize() + sizeof(long long);
  char* buffer = (char*)malloc(count * dataSize);
  IRequest_ptr ptr = comm->IRecv(buffer, count, dest, tag, dataSize);
  ptr->buffer = buffer;
  ptr->count = count;
  ptr->recv = true;
  return ptr;
}

std::pair<IDataType_vec, IStatus_ptr> DataTypeCommunication::Recv(int source,
                                                                  int tag) {
  // Recv without knowing the DataType.
  IStatus_ptr status = Probe(source, tag);
  // Get the size of the message in bytes. Because we don't know the data type,
  // we will have to recieve this one in bytes.
  int byteSize = comm->Count(status, sizeof(char));
  char* buffer = (char*)malloc(byteSize);
  status = comm->Recv(buffer, byteSize, status->source(), status->tag(),
                      sizeof(char));

  IDataType_vec results;
  if (byteSize > sizeof(long long)) {
    long long* buff = (long long*)buffer;
    long long dataType = buff[0];
    IDataType_ptr ptr = DataTypeStore::instance().getDataType(dataType);
    long dataSize = ptr->maxSize() + sizeof(long long);
    if (byteSize % dataSize != 0) {
      throw VnV::VnVExceptionBase(
          "Invalid Recv Data Found. Recv is not some multiple of data size. ");
    }
    int count = byteSize / dataSize;
    for (int i = 0; i < count; i++) {
      IDataType_ptr ptr = DataTypeStore::instance().getDataType(dataType);
      long long* buff = (long long*)&(buffer[i * dataSize]);
      ptr->unpack(&(buff[1]));
      results.push_back(ptr);
    }
    free(buffer);
  } else if (byteSize == 0) {
  } else {
    throw VnV::VnVExceptionBase(
        "Invalid message recieved -- message to small to be from VnV");
  }
  return std::make_pair(results, status);
}

IDataType_vec DataTypeCommunication::BroadCast(IDataType_vec& data, long long dtype, int count,
                                               int root, bool allToAll) {
  int rank = comm->Rank();
  int size = comm->Size();

  // First, need to broadcast the data type.
  long long dataSize = DataTypeStore::instance().getDataType(dtype)->maxSize();

  // Now pop the send buffer
  char* sendBuffer;
  sendBuffer = (char*)malloc(data.size() * dataSize);

  if (rank == root || allToAll) {
    for (std::size_t i = 0; i < data.size(); i++) {
      data[i]->pack(&(sendBuffer[i * dataSize]));
    }
  }

  // Call the methods
  IDataType_vec results;
  if (allToAll) {
    results.reserve(size * count);
    char* recvBuffer = (char*)malloc(size * count * dataSize);
    comm->AllToAll(sendBuffer, count, recvBuffer, dataSize);
    free(sendBuffer);

    // Unwrap.
    for (int i = 0; i < size * count; i++) {
      IDataType_ptr ptr = DataTypeStore::instance().getDataType(dtype);
      ptr->unpack(&(recvBuffer[i * dataSize]));
      results.push_back(ptr);
    }
    free(recvBuffer);
  } else {
    results.reserve(count);
    comm->BroadCast(sendBuffer, count, dataSize, root);

    // Unwrap
    for (int i = 0; i < count; i++) {
      IDataType_ptr ptr = DataTypeStore::instance().getDataType(dtype);
      ptr->unpack(&(sendBuffer[i * dataSize]));
      results.push_back(ptr);
    }
    free(sendBuffer);
  }
  return results;
}

IDataType_vec DataTypeCommunication::GatherV(IDataType_vec &data, long long dtype, int root, std::vector<int> &gsizes, std::vector<int> &sizes, std::vector<int> &offsets, bool allGather) {
   int dim = offsets.size();
   int csize = 1 + 2*dim;

   std::vector<int> sendBuffer;
   sendBuffer.reserve(csize);
   sendBuffer.push_back(data.size());
   for (int i = 0; i < dim; i++ ){
     sendBuffer.push_back(sizes[i]);
     sendBuffer.push_back(offsets[i]);
   }


   int total = 0;
   std::vector<int> result((comm->Rank()==root || allGather ) ? comm->Size() * csize  : 0 );
   std::vector<int> counts((comm->Rank() ==root || allGather) ? comm->Size() : 0 );
   std::vector<int> displs((comm->Rank() ==root || allGather) ? comm->Size() : 0 );

   if (!allGather) {
      comm->Gather(&(sendBuffer[0]), csize, &(result[0]), sizeof(int), root);
   } else {
      comm->AllGather(&(sendBuffer[0]), csize, &(result[0]), sizeof(int));
   }

   // displs is the displacement where we put the vector data in. Count is
   // the number from each process.

   // results = [[ count , xdim, xoff, ydim, yoff, zdim, zoff, ...] ,[...]]
   for (int i =0; i < counts.size(); i++ )  {
     displs[i] = total;
     total += result[i*csize];
     counts[i] = result[i*csize];
   }

   // Pop the send buffer -- Everyone has one already,
   char* sendbuffer = nullptr;
   auto it = DataTypeStore::instance().getDataType(dtype);

   long long dataSize = it->maxSize();
   sendbuffer = (char*)malloc(dataSize * data.size());
   for (std::size_t i = 0; i < data.size(); i++) {
     data[i]->pack(&(sendbuffer[i * dataSize]));
  }

  // Alloc the recv buffer on any rank that needs it.
  char* recvBuffer;
  if (comm->Rank() == root || allGather) {
    recvBuffer = (char*)malloc(dataSize * total );
  }

  // Call the Gather
  if (allGather) {
    comm->AllGatherV(sendbuffer, data.size(), recvBuffer, &counts[0], &(displs[0]), dataSize);
  } else {
    comm->GatherV(sendbuffer, data.size(), recvBuffer, &(counts[0]), &(displs[0]), dataSize, root);
  }
  free(sendbuffer);

  // Unwrap into a 1-hot excoded structure.
  // A_{ijk...} = vec[i*xs + j*ys + ...]
  // Result = [ [total, xsize, xoff, ysize, yoff, zsize, zoff...] repeat]

  IDataType_vec results(total);

  if (comm->Rank() == root || allGather) {
     //Get the multipliers for the flatter operation.
     int cc = 0;
     std::vector<int> multipliers(dim);
     for (int i = dim; i > 0 ; i--) {
        multipliers[i-1] = (i==dim) ? 1 : multipliers[i]*gsizes[cc++];
     }

    // Iterate over all communicators.
    int cindex = 0; // current index in the recv buffer.
    for ( int i =0; i < comm->Size(); i++) {

      //Get the sizes and offsets for this vector.
      std::vector<int> sizes(dim);
      std::vector<int> offs(dim);
      for (int k = 0; k < dim; k++) {
        sizes[k] = ( result[i*csize + 1 + 2*k]);
        offs[k] = ( result[i*csize + 2 + 2*k]);
      }

      std::vector<int> iters(dim,0);
      //Loop over all the vectors from this comm.
      for( int j = 0; j < counts[i]; j++ ) {

          // J IS THE A_{ijk} -> V[j]. Now need to map l-> {ijk...}
          int findex = 0;
          bool up = false;
          for (int jj = dim-1; jj >= 0 ; jj-- ) {
              findex += (offs[jj]+iters[jj])*multipliers[jj];
              if (!up) {
                 iters[jj] = (iters[jj] + 1) % sizes[jj];
                 up = (iters[jj] > 0 ) ; // if iters[jj] == 0 then next one should increase
              }
          }

          //Finally unpack the sucker.
          IDataType_ptr dptr = DataTypeStore::instance().getDataType(dtype);
          dptr->unpack(&(recvBuffer[cindex * dataSize]));
          results[findex] = dptr;

          cindex++; // Move to the next value in the main unpacking vector.
      }

    }

  }
  if (comm->Rank() == root || allGather) {
    free(recvBuffer);
  }
  return results;
}

IDataType_vec DataTypeCommunication::Gather(IDataType_vec& data, long long dtype, int root,
                                            bool allGather) {
  int rank = comm->Rank();
  int size = comm->Size();

  // Pop the send buffer -- Everyone has one already,
  char* sendbuffer = nullptr;

  long long dataSize = DataTypeStore::instance().getDataType(dtype)->maxSize();
  sendbuffer = (char*)malloc(dataSize * data.size());
  for (std::size_t i = 0; i < data.size(); i++) {
    data[i]->pack(&(sendbuffer[i * dataSize]));
  }

  // Alloc the recv buffer on any rank that needs it.
  char* recvBuffer;
  if (rank == root || allGather) {
    recvBuffer = (char*)malloc(dataSize * data.size() * size);
  }

  // Call the Gather
  if (allGather) {
    comm->AllGather(sendbuffer, data.size(), recvBuffer, dataSize);
  } else {
    comm->Gather(sendbuffer, data.size(), recvBuffer, dataSize, root);
  }

  // Unwrap
  IDataType_vec results;
  results.reserve(dataSize * data.size());
  if (rank == root || allGather) {
    for (int i = 0; i < dataSize * data.size(); i++) {
      IDataType_ptr dptr = DataTypeStore::instance().getDataType(dtype);
      dptr->unpack(&(recvBuffer[i * dataSize]));
      results.push_back(dptr);
    }
    free(recvBuffer);
  }
  free(sendbuffer);
  return results;
}

IDataType_vec DataTypeCommunication::Scatter(IDataType_vec& data, long long dtype, int root,
                                             int count) {
  int rank = comm->Rank();
  int size = comm->Size();

  long dataSize = DataTypeStore::instance().getDataType(dtype)->maxSize();

  // Now pop the send buffer
  char* buffer = nullptr;
  if (rank == root && data.size() != count * size) {
    buffer = (char*)malloc(dataSize * data.size());
    for (int i = 0; i < data.size(); i++) {
      data[i]->pack(&(buffer[i * dataSize]));
    }
  }

  // Alloc the recv buffer and perform the scatter.
  char* recvBuffer = (char*)malloc(count * dataSize);
  comm->Scatter(buffer, count, recvBuffer, dataSize, root);

  // Unwrap.
  IDataType_vec results;
  for (int i = 0; i < count; i++) {
    IDataType_ptr dptr = DataTypeStore::instance().getDataType(dtype);
    dptr->unpack(&(recvBuffer[i * dataSize]));
    results.push_back(dptr);
  }

  // Free
  free(recvBuffer);
  if (rank == root) {
    free(buffer);
  }
  return results;
}

IDataType_vec DataTypeCommunication::ReduceMultiple(IDataType_vec &data, long long dtype, std::string packageColonName, int root) {
  return ReduceMultiple(data, dtype, ReductionStore::instance().getReducer(packageColonName),root);
}

IDataType_ptr DataTypeCommunication::ReduceLocalVec(IDataType_vec &data, IReduction_ptr reducer) {
   if (data.size() == 0 ) return nullptr;

   IDataType_ptr result = data[0];
   for (int i = 1; i < data.size(); i++) {
      result = reducer->reduce(data[i],result);
   }
   return result;
}

IDataType_vec DataTypeCommunication::ReduceMultiple(IDataType_vec& data, long long dtype,
                                            IReduction_ptr reduction,
                                            int root) {
  int rank = comm->Rank();

  // Pop the send buffer
  long long reducerKey = reduction->getKey();
  long dataSize = DataTypeStore::instance().getDataType(dtype)->maxSize() + 2 * sizeof(long long);
  char* buffer = (char*)malloc(data.size() * dataSize);
  for (int i = 0; i < data.size(); i++) {
    long long* lptr = (long long*) &(buffer[i * dataSize]);
    lptr[0] = reducerKey;       // reducer
    lptr[1] = dtype;          // key
    data[i]->pack(&(lptr[2]));  // data
  }

  // Call the reduction operation
  OpType op = (reduction->communitive()) ? OpType::ENCODED_COMMUTE
                                         : OpType::ENCODED_NONCOMMUTE;
  char* recvBuffer = (root < 0 || rank == root)
                              ? (char*) malloc(data.size() * dataSize)
                              : nullptr;
  if (root == -1) {
    comm->AllReduce(buffer, data.size(), recvBuffer, dataSize, op);
  } else if (root == -2) {
    comm->Scan(buffer, data.size(), recvBuffer, dataSize, op);
  } else {
    comm->Reduce(buffer, data.size(), recvBuffer, dataSize, op, root);
  }

  // Unpack.
  IDataType_vec results;
  results.reserve(data.size());
  if (root < 0 || rank == root) {
    for (int i = 0; i < data.size(); i++) {
      long long* lptr = (long long*) &(recvBuffer[i * dataSize]);
      IDataType_ptr result =
          DataTypeStore::instance().getDataType(dtype);
      result->unpack(&(lptr[2]));
      results.push_back(result);
    }
    free(recvBuffer);
  }
  free(buffer);
  return results;
}
