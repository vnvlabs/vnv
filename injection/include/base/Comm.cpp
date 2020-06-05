
namespace VnV {
namespace Communication {

class DataTypeCommunication {
  ICommunicator_ptr comm;

  IStatus_ptr Probe(int source, int tag) { return comm->Probe(source, tag); }
  std::pair<IStatus_ptr, int> IProbe(int source, int tag) {
    return comm->IProbe(source, tag);
  }

  IStatus_ptr Wait(IRequest_ptr ptr) {
    auto s = comm->Wait(ptr);
    s->ready = true;
    return s;
  }
  IStatus_vec WaitAll(IRequest_vec& vec) {
    auto s = comm->WaitAll(vec);
    for (auto it : s) it->ready = true;
    return s;
  }
  std::pair<IStatus_vec, int> WaitAny(IRequest_vec& vec) {
    auto s = comm->WaitAny(vec);
    s.first[s.second]->ready = true;
    return s;
  }
  std::pair<IStatus_ptr, int> Test(IRequest_ptr ptr) {
    auto s = comm->Test(ptr);
    if (s.second) {
      s.first->ready = true;
    }
    return s;
  }
  std::vector<std::pair<IStatus_ptr, int>> TestAll(IRequest_vec& vec) {
    auto s = comm->TestAll(vec);
    for (auto& it : s) {
      if (it.second) it.first->ready = true;
    }
    return s;
  }
  std::pair<std::vector<std::pair<IStatus_ptr, int>>, int> TestAny(
      IRequest_vec& vec) {
    auto s = comm->TestAny(vec);
    if (s.first[s.second].second) {
      s.first[s.second].first->ready = true;
    }
    return s;
  }

  ISendRequest_ptr Send(IDataType_vec& data, int dest, int tag, bool blocking) {
    long dataSize = data[0]->maxSize() + sizeof(long long);
    void* buffer = malloc(dataSize * data.size());
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
      ISendRequest_ptr ptr =
          comm->ISend(buffer, data.size(), dest, tag, dataSize);
      ptr->buffer(buffer);  // set the buffer so we can free on destruction.
      return ptr;
    }
  }

  std::pair<IDataType_vec, IStatus_ptr> Recv(int count, long long dataType,
                                             int dest, int tag) {
    IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
    long dataSize = ptr->maxSize() + sizeof(long long);
    void* buffer = malloc(count * dataSize);
    comm->Recv(buffer, count, dest, tag, dataSize);

    // Unwrap.
    IDataType_vec results;
    for (int i = 0; i < count; i++) {
      IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
      long long* buff = (long long*)&(buffer[i * dataSize]);
      ptr->unpack(&(buff[1]));
      results.push_back(ptr);
    }
    free(buffer);
    return results;
  }

  // Recieve a data type when size and count are known.
  IRecvRequest_ptr IRecv(int count, long long dataType, int dest, int tag) {
    IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
    long dataSize = ptr->maxSize() + sizeof(long long);
    void* buffer = malloc(count * dataSize);
    IRecvRequest_ptr ptr = comm->Recv(buffer, count, dest, tag, dataSize);
    ptr->buffer = buffer;
    ptr->count = count;
    return ptr;
  }

  // Recv an arbitary data type without knowing what it is.
  std::pair<IDataType_vec, IStatus_ptr> Recv(int dest, int tag) {
    // Recv without knowing the DataType.
    IStatus_ptr status = Probe(source, tag);
    // Get the size of the message in bytes. Because we don't know the data
    // type, we will have to recieve this one in bytes.
    int byteSize = comm->Count(status, sizeof(char));
    void* buffer = malloc(byteSize);
    comm->Recv(buffer, byteSize, dest, tag, sizeof(char));

    IDataType_vec results;
    if (byteSize > sizeof(long long)) {
      long long* buff = (long long*)buffer;
      long long dataType = buff[0];
      IDataType_ptr ptr = CommunicationStore::instance().getDataType(dataType);
      long dataSize = ptr->maxSize() + sizeof(long long);
      if (byteSize % dataSize != 0) {
        throw VnV::VnVExceptionBase(
            "Invalid Recv Data Found. Recv is not some multiple of data "
            "size. ");
      }
      int count = byteSize / dataSize;
      for (int i = 0; i < count; i++) {
        IDataType_ptr ptr =
            CommunicationStore::instance().getDataType(dataType);
        long long* buff = (long long*)&(buffer[i * dataSize]);
        ptr->unpack(&(buff[1]));
        results.push_back(ptr);
      }
      free(buffer);
      return results;
    } else if (byteSize == 0) {
      return results;  // empty message;
    } else {
      throw VnV::VnVExceptionBase(
          "Invalid message recieved -- message to small to be from VnV");
    }
  }

  IDataType_vec BroadCast(IDataType_vec& data, int count, int root,
                          bool allToAll) {
    int rank = comm->Rank();
    int size = comm->Size();

    // First, need to broadcast the data type.
    long long key = (rank == root) ? data[0]->getKey() : 0;
    long long recvKey = -1;
    comm->BroadCast(&key, 1, &recvKey, sizeof(long long), root);
    long dataSize =
        CommunicationStore::instance().getDataType(recvKey)->maxSize();

    // Now pop the send buffer
    void* sendBuffer;
    if (rank == root || allToAll) {
      long dataSize = data[0]->maxSize();
      sendBuffer = malloc(data.size() * dataSize);
      for (int i = 0; i < data.size(); i++) {
        data[i]->pack(&(sendBuffer[i * dataSize]));
      }
    }

    // Call the methods
    IDataType_vec results;
    if (allToAll) {
      results.reserve(size * count);
      recvBuffer = malloc(size * count * dataSize);
      comm->AllToAll(sendBuffer, count, recvBuffer, dataSize);
      free(sendBuffer);

      // Unwrap.
      for (int i = 0; i < size * count; i++) {
        IDataType_ptr ptr = CommunicationStore::instance().getDataType(recvKey);
        ptr->unpack(&(recvBuffer[i * dataSize]));
        results.push_back(ptr);
      }
      free(recvBuffer);
    } else {
      results.reserve(count);
      recvBuffer = malloc(count * datasize);
      comm->BroadCast(sendBuffer, count, recvBuffer, dataSize, root);
      free(sendBuffer);

      // Unwrap
      for (int i = 0; i < count; i++) {
        IDataType_ptr ptr = CommunicationStore::instance().getDataType(recvKey);
        ptr->unpack(&(recvBuffer[i * dataSize]));
        results.push_back(ptr);
      }
      free(recvBuffer);
    }
    return results;
  }

  IDataType_vec Gather(IDataType_vec& data, int root, bool allGather) {
    int rank = comm->Rank();
    int size = comm->Size();

    // Pop the send buffer -- Everyone has one already,
    void* sendbuffer = nullptr;
    long long key = data[0]->getKey();
    long dataSize = data[0]->maxSize();
    buffer = malloc(dataSize * data.size());
    for (int i = 0; i < data.size(); i++) {
      it->pack(&(buffer[i * dataSize]));
    }

    // Alloc the recv buffer on any rank that needs it.
    void* recvBuffer;
    if (rank == root || allGather) {
      recvBuffer = malloc(dataSize * data.size() * size);
    }

    // Call the Gather
    if (allGather) {
      comm->AllGather(sendbuffer, recvBuffer, data.size(), dataSize);
    } else {
      comm->Gather(sendbuffer, recvBuffer, data.size(), dataSize, root);
    }

    // Unwrap
    IDataType_vec results;
    results.reserve(dataSize * data.size();) if (rank == root || allGather) {
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

  IDataType_vec Scatter(IDataType_vec& data, int root, int count) {
    int rank = comm->Rank();
    int size = comm->Size();

    // First, need to scatter the data type.
    long long key = (rank == root) ? data[0]->getKey() : 0;
    long long recvKey = -1;
    comm->BroadCast(&key, 1, &recvKey, sizeof(long long), root);
    long dataSize =
        CommunicationStore::instance().getDataType(recvKey)->maxSize();

    // Now pop the send buffer
    void* buffer = nullptr;
    if (rank == root && data.size() != count * size) {
      buffer = malloc(dataSize * data.size());
      for (int i = 0; i < data.size(); i++) {
        it->pack(&(buffer[i * dataSize]));
      }
    }

    // Alloc the recv buffer and perform the scatter.
    void* recvBuffer = malloc(count * dataSize);
    comm->Scatter(buffer, count, recvBuffer, dataSize, root);

    // Unwrap.
    IDataType_vec results;
    for (int i = 0; i < count; i++) {
      long long* lptr = (long long*)&(recvBuffer[i * dataSize]);
      IDataType_ptr dptr = CommunicationStore::instance().getDataType(recvKey);
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

  // TODO Assumes Datatype vec are all the same type.
  IDataType_vec Reduce(IDataType_vec& data, IReduction_ptr reduction,
                       int root) {
    int rank = comm->Rank();

    // Pop the send buffer
    long long dataKey = data[0]->getKey();
    long long reducerKey = reduction->getKey();
    long dataSize = data[0]->maxSize() + 2 * sizeof(long long);
    void* buffer = malloc(data.size() * dataSize);
    for (int i = 0; i < data.size(); i++) {
      long long* lptr = (long long*)&(buffer[i * dataSize]);
      lptr[0] = reducerKey;  // reducer
      lptr[1] = dataKey;     // key
      it->pack(&(lptr[2]));  // data
    }

    // Call the reduction operation
    OpType op = (reduction->communitive()) ? OpType::ENCODED_COMMUTE
                                           : OpType::ENCODED_NONCOMMUTE;
    long long* recvBuffer =
        (root < 0 || rank == root) ? (long long*)malloc(buffSize) : nullptr;
    if (root == -1) {
      comm->AllReduce(buffer, vec.size(), recvBuffer, dataSize, op);
    } else if (root == -2) {
      comm->Scan(buffer, vec.size(), recvBuffer, dataSize, op);
    } else {
      comm->Reduce(buffer, vec.size(), recvBuffer, dataSize, root, op);
    }

    // Unpack.
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
};

}  // namespace Communication
}  // namespace VnV
