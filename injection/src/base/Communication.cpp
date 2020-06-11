#include "base/Communication.h"
#include "base/CommunicationStore.h"
#include "base/exceptions.h"

using namespace VnV::Communication;

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
    for (int i = 0; i < s.first.size(); i++) {
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
  IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
  long dataSize = ptr->maxSize() + sizeof(long long);
  char* buffer = (char*)malloc(count * dataSize);
  IStatus_ptr status = comm->Recv(buffer, count, dest, tag, dataSize);

  // Unwrap.
  IDataType_vec results;
  for (int i = 0; i < count; i++) {
    IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
    long long* buff = (long long*)&(buffer[i * dataSize]);
    ptr->unpack(&(buff[1]));
    results.push_back(ptr);
  }
  free(buffer);
  return std::make_pair(results, status);
}

IRequest_ptr DataTypeCommunication::IRecv(int count, long long dataType,
                                          int dest, int tag) {
  IDataType_ptr dptr = CommunicationStore::instance().getDataType(dataType);
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
    IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
    long dataSize = ptr->maxSize() + sizeof(long long);
    if (byteSize % dataSize != 0) {
      throw VnV::VnVExceptionBase(
          "Invalid Recv Data Found. Recv is not some multiple of data size. ");
    }
    int count = byteSize / dataSize;
    for (int i = 0; i < count; i++) {
      IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
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

IDataType_vec DataTypeCommunication::BroadCast(IDataType_vec& data, int count,
                                               int root, bool allToAll) {
  int rank = comm->Rank();
  int size = comm->Size();

  // First, need to broadcast the data type.
  long long key = (rank == root) ? data[0]->getKey() : -1;
  comm->BroadCast(&key, 1, sizeof(long long), root);
  long dataSize = CommunicationStore::instance().getDataType(key)->maxSize();

  // Now pop the send buffer
  char* sendBuffer;
  sendBuffer = (char*)malloc(data.size() * dataSize);

  if (rank == root || allToAll) {
    for (int i = 0; i < data.size(); i++) {
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
      IDataType_ptr ptr = CommunicationStore::instance().getDataType(key);
      ptr->unpack(&(recvBuffer[i * dataSize]));
      results.push_back(ptr);
    }
    free(recvBuffer);
  } else {
    results.reserve(count);
    comm->BroadCast(sendBuffer, count, dataSize, root);

    // Unwrap
    for (int i = 0; i < count; i++) {
      IDataType_ptr ptr = CommunicationStore::instance().getDataType(key);
      ptr->unpack(&(sendBuffer[i * dataSize]));
      results.push_back(ptr);
    }
    free(sendBuffer);
  }
  return results;
}

IDataType_vec DataTypeCommunication::Gather(IDataType_vec& data, int root,
                                            bool allGather) {
  int rank = comm->Rank();
  int size = comm->Size();

  // Pop the send buffer -- Everyone has one already,
  char* sendbuffer = nullptr;
  long long key = data[0]->getKey();
  long dataSize = data[0]->maxSize();
  sendbuffer = (char*)malloc(dataSize * data.size());
  for (int i = 0; i < data.size(); i++) {
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
      IDataType_ptr dptr = CommunicationStore::instance().getDataType(key);
      dptr->unpack(&(recvBuffer[i * dataSize]));
      results.push_back(dptr);
    }
    free(recvBuffer);
  }
  free(sendbuffer);
  return results;
}

IDataType_vec DataTypeCommunication::Scatter(IDataType_vec& data, int root,
                                             int count) {
  int rank = comm->Rank();
  int size = comm->Size();

  // First, need to scatter the data type.
  long long key = (rank == root) ? data[0]->getKey() : 0;
  comm->BroadCast(&key, 1, sizeof(long long), root);
  long dataSize = CommunicationStore::instance().getDataType(key)->maxSize();

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
    long long* lptr = (long long*)&(recvBuffer[i * dataSize]);
    IDataType_ptr dptr = CommunicationStore::instance().getDataType(key);
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

IDataType_vec DataTypeCommunication::Reduce(IDataType_vec& data,
                                            IReduction_ptr reduction,
                                            int root) {
  int rank = comm->Rank();

  // Pop the send buffer
  long long dataKey = data[0]->getKey();
  long long reducerKey = reduction->getKey();
  long dataSize = data[0]->maxSize() + 2 * sizeof(long long);
  char* buffer = (char*)malloc(data.size() * dataSize);
  for (int i = 0; i < data.size(); i++) {
    long long* lptr = (long long*)&(buffer[i * dataSize]);
    lptr[0] = reducerKey;       // reducer
    lptr[1] = dataKey;          // key
    data[i]->pack(&(lptr[2]));  // data
  }

  // Call the reduction operation
  OpType op = (reduction->communitive()) ? OpType::ENCODED_COMMUTE
                                         : OpType::ENCODED_NONCOMMUTE;
  long long* recvBuffer = (root < 0 || rank == root)
                              ? (long long*)malloc(data.size() * dataSize)
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
  if (root < 0 || rank == root) {
    for (int i = 0; i < data.size(); i++) {
      long long* lptr = (long long*)recvBuffer[i * dataSize];
      IDataType_ptr result =
          CommunicationStore::instance().getDataType(dataKey);
      result->unpack(&(lptr[2]));
      results.push_back(result);
      free(recvBuffer);
    }
  }
  free(buffer);
  return results;
}
