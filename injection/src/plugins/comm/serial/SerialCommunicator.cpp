
#include <stdio.h>

#include <cstring>
#include <list>

#include "base/exceptions.h"
#include "interfaces/ICommunicator.h"

using namespace VnV::Communication;

class SerialStatus : public VnV::Communication::IStatus {
 public:
  int tagN;
  long size;
  SerialStatus(int tag, long size) {
    tagN = tag;
    this->size = size;
  }
  int source() { return 0; }
  int tag() { return tagN; }
  int error() { return -1; }

  long count(long dataTypeSize) { return size / dataTypeSize; }
};

class SerialRequest : public VnV::Communication::IRequest {
  static long long idCounter;

 public:
  bool sent_or_recvd = false;
  long long m_id;
  void* m_buffer;
  long m_size;
  int m_tag;
  bool m_del;
  SerialRequest(void* buffer, long size, int tag, bool del)
      : m_buffer(buffer), m_size(size), m_tag(tag), m_del(del) {
    m_id = idCounter++;
  }

  ~SerialRequest() {
    if (m_del) {
      free(m_buffer);
    }
  }
};
long long SerialRequest::idCounter = 0;

typedef std::shared_ptr<SerialRequest> SerialRequest_ptr;
SerialRequest_ptr getSR(void* buffer, long size, int tag, bool del) {
  return std::make_shared<SerialRequest>(buffer, size, tag, del);
}

IStatus_ptr getSP(int tag, long size) {
  return IStatus_ptr(new SerialStatus(tag, size));
}

class SerialCommunicator : public VnV::Communication::ICommunicator {
  static int communicatorCount;
  static long long commCount;
  static std::map<int, std::list<SerialRequest_ptr>> sendqueue;
  static std::map<int, std::list<SerialRequest_ptr>> recvqueue;

  int commId = 0;
  // ICommunicator interface
 public:
  void Process() {
    auto sendQueue = getSendQueue();
    auto recvQueue = getRecvQueue();

    if (sendQueue.size() == 0 || recvQueue.size() == 0) {
      return;
    }  // no sends to process or no recv placed

    auto recvIt = recvQueue.begin();
    auto sendIt = sendQueue.begin();
    while (sendIt != sendQueue.end()) {
      int flag = 0;
      while (flag == 0) {
        if (recvIt == recvQueue.end()) {
          flag = 1;
        } else if ((*recvIt)->m_tag == (*sendIt)->m_tag ||
                   (*recvIt)->m_tag == -1) {
          // Someone else was in line first -- so, read, erase, move on.
          memcpy((*recvIt)->m_buffer, (*sendIt)->m_buffer, (*recvIt)->m_size);
          (*recvIt)->sent_or_recvd = true;
          (*sendIt)->sent_or_recvd = true;
          (*recvIt)->m_tag = (*sendIt)->m_tag;
          recvQueue.erase(recvIt);
          sendIt = sendQueue.erase(sendIt);
          recvIt = recvQueue.begin();  // go back to start TODO
          flag = 1;
        } else {
          recvIt++;
        }
      }
      sendIt++;  // No recv for that send yet.
    }
  }

  SerialCommunicator() { commId = communicatorCount++; }

  std::list<SerialRequest_ptr>& getSendQueue() {
    auto it = sendqueue.find(commId);
    if (it == sendqueue.end()) {
      std::list<SerialRequest_ptr> l;
      sendqueue.insert(std::make_pair(commId, l));
      return sendqueue[commId];
    } else {
      return it->second;
    }
  }

  std::list<SerialRequest_ptr>& getRecvQueue() {
    auto it = recvqueue.find(commId);
    if (it == sendqueue.end()) {
      std::list<SerialRequest_ptr> l;
      sendqueue.insert(std::make_pair(commId, l));
      return sendqueue[commId];
    } else {
      return it->second;
    }
  }

  void setData(void* data) { commId = *((int*)data); }
  void* getData() { return &commId; }
  int uniqueId() { return (int)(size_t)commId; }
  int Size() { return 1; }
  int Rank() { return 0; }
  void Barrier() {}
  std::string ProcessorName() { return std::to_string(commId); }
  double time() { return -1; }
  double tick() { return -1; }
  VnV::Communication::ICommunicator_ptr duplicate() {
    return std::make_shared<SerialCommunicator>();
  }
  VnV::Communication::ICommunicator_ptr split(int color, int key) {
    return duplicate();
  }
  VnV::Communication::ICommunicator_ptr create(std::vector<int>& ranks,
                                               int stride) {
    return duplicate();
  }
  VnV::Communication::ICommunicator_ptr create(int start, int end, int stride,
                                               int tag) {
    return duplicate();
  }

  CommCompareType compare(ICommunicator_ptr ptr) {
    return (commId == *((int*)ptr->getData())) ? CommCompareType::EXACT
                                               : CommCompareType::UNEQUAL;
  }

  // Only contains if it is itself.
  bool contains(ICommunicator_ptr ptr) {
    return compare(ptr) == CommCompareType::EXACT;
  }

  void Send(void* buffer, int count, int dest, int tag, int dataTypeSize) {
    auto q = getSendQueue();
    void* data = malloc(count * dataTypeSize);
    memcpy(data, buffer, count * dataTypeSize);
    auto mess = getSR(data, count * dataTypeSize, tag, true);
    q.push_back(mess);
  }

  VnV::Communication::IRequest_ptr ISend(void* buffer, int count, int dest,
                                         int tag, int dataTypeSize) {
    // ISend means we dont need to copy the buffer we just add the message.

    auto q = getSendQueue();
    auto mess = getSR(buffer, count * dataTypeSize, tag, false);
    q.push_back(mess);
    return mess;
  }

  VnV::Communication::IStatus_ptr Recv(void* buffer, int count, int dest,
                                       int tag, int dataTypeSize) {
    auto s = getSR(buffer, count * dataTypeSize, tag, false);
    getRecvQueue().push_back(s);
    Process();
    if (!s->sent_or_recvd) {
      throw VnV::VnVExceptionBase("No send found for this recv");
    }
    return getSP(s->m_tag, s->m_size);
  }
  VnV::Communication::IRequest_ptr IRecv(void* buffer, int count, int dest,
                                         int tag, int dataTypeSize) {
    auto s = getSR(buffer, count * dataTypeSize, tag, false);
    getRecvQueue().push_back(s);
    return s;
  }

  VnV::Communication::IStatus_ptr Wait(VnV::Communication::IRequest_ptr ptr) {
    SerialRequest* s = (SerialRequest*)ptr.get();
    if (!s->sent_or_recvd) {
      Process();
    }
    if (!s->sent_or_recvd) {
      VnV::VnVExceptionBase("No send for this request");
    }
    return getSP(s->m_tag, s->m_size);
  }

  VnV::Communication::IStatus_vec WaitAll(
      VnV::Communication::IRequest_vec& vec) {
    std::vector<SerialRequest*> r(vec.size());
    std::vector<IStatus_ptr> res(vec.size());
    bool needsProcess = false;
    for (auto it : vec) {
      r.push_back((SerialRequest*)it.get());
      if (!r.back()->sent_or_recvd) {
        needsProcess = true;
      }
    }
    if (needsProcess) Process();

    for (auto it : r) {
      if (!it->sent_or_recvd) {
        throw VnV::VnVExceptionBase("No send for a Wait --");
      } else {
        res.push_back(getSP(it->m_tag, it->m_size));
      }
    }
    return res;
  }

  std::pair<VnV::Communication::IStatus_ptr, int> WaitAny(
      VnV::Communication::IRequest_vec& vec) {
    std::vector<SerialRequest*> r(vec.size());
    std::vector<IStatus_ptr> res(vec.size());
    int count = 0;
    for (auto it : vec) {
      r.push_back((SerialRequest*)it.get());
      if (r.back()->sent_or_recvd) {
        auto s = getSP(r.back()->m_tag, r.back()->m_size);
        return std::make_pair(s, count);
      }
      count++;
    }

    Process();
    count = 0;
    for (auto it : r) {
      if (it->sent_or_recvd) {
        auto s = getSP(it->m_tag, it->m_size);
        return std::make_pair(s, count);
      }
      count++;
    }
    throw VnV::VnVExceptionBase("No Wait was successfull");
  }

  int Count(VnV::Communication::IStatus_ptr status, int dataTypeSize) {
    return ((SerialStatus*)status.get())->count(dataTypeSize);
  }

  VnV::Communication::IStatus_ptr Probe(int source, int tag) {
    Process();  // clear a send list where possible.
    auto& sendQ = getSendQueue();
    for (auto& it : sendQ) {
      if (it->m_tag == tag || tag == -1) {
        return getSP(it->m_tag, it->m_size);
      }
    }
    throw VnV::VnVExceptionBase("Probe with no matching send");
  }

  std::pair<VnV::Communication::IStatus_ptr, int> IProbe(int source, int tag) {
    Process();  // clear a send list where possible.
    auto& sendQ = getSendQueue();
    int count = 0;
    for (auto& it : sendQ) {
      if (it->m_tag == tag || tag == -1) {
        return std::make_pair(getSP(it->m_tag, it->m_size), 1);
      }
      count++;
    }
    return std::make_pair<VnV::Communication::IStatus_ptr, int>(nullptr, 0);
  }

  std::pair<VnV::Communication::IStatus_ptr, int> Test(
      VnV::Communication::IRequest_ptr ptr) {
    SerialRequest* r = (SerialRequest*)ptr.get();
    if (r->sent_or_recvd)
      return std::make_pair(getSP(r->m_tag, r->m_size), true);
    Process();
    return std::make_pair(getSP(r->m_tag, r->m_size), r->sent_or_recvd);
  }
  std::pair<VnV::Communication::IStatus_vec, int> TestAll(
      VnV::Communication::IRequest_vec& vec) {
    Process();
    IStatus_vec res;
    int f = 1;
    for (auto it : vec) {
      SerialRequest* r = (SerialRequest*)it.get();
      res.push_back(getSP(r->m_tag, r->m_size));
      if (!r->sent_or_recvd) f = 0;
    }
    return std::make_pair(res, f);
  }

  std::tuple<VnV::Communication::IStatus_ptr, int, int> TestAny(
      VnV::Communication::IRequest_vec& vec) {
    Process();
    int f = 0;
    for (auto it : vec) {
      SerialRequest* r = (SerialRequest*)it.get();
      if (r->sent_or_recvd) {
        return {getSP(r->m_tag, r->m_size), f, true};
      }
    }
    return {nullptr, -1, false};
  }

  // All of the comm calls just copy the send buffer to the recv buffer.
  void Gather(void* buffer, int count, void* recvBuffer, int dataTypeSize,
              int root) {
    if (recvBuffer != buffer) {
      memcpy(recvBuffer, buffer, count * dataTypeSize);
    }
  }

  void AllGather(void* buffer, int count, void* recvBuffer, int dataTypeSize) {
    Gather(buffer, count, recvBuffer, dataTypeSize, 0);
  }
  void BroadCast(void* buffer, int count, int dataTypeSize, int root) {
    // Gather(buffer,count, recvBuffer,dataTypeSize,0);
  }
  void AllToAll(void* buffer, int count, void* recvBuffer, int dataTypeSize) {
    Gather(buffer, count, recvBuffer, dataTypeSize, 0);
  }

  void Scatter(void* buffer, int count, void* recvBuffer, int dataTypeSize,
               int root) {
    Gather(buffer, count, recvBuffer, dataTypeSize, 0);
  }

  void Reduce(void* buffer, int count, void* recvBuffer, int dataTypeSize,
              VnV::Communication::OpType op, int root) {
    Gather(buffer, count, recvBuffer, dataTypeSize, 0);
  }

  void AllReduce(void* buffer, int count, void* recvBuffer, int dataTypeSize,
                 VnV::Communication::OpType op) {
    Reduce(buffer, count, recvBuffer, dataTypeSize, op, 0);
  }

  void Scan(void* buffer, int count, void* recvBuffer, int dataTypeSize,
            VnV::Communication::OpType op) {
    Reduce(buffer, count, recvBuffer, dataTypeSize, op, 0);
  }

  void Abort(int errorcode) { VnV::VnVExceptionBase("Serial Abort called"); }
};

int SerialCommunicator::communicatorCount = 0;
long long SerialCommunicator::commCount = 0;
std::map<int, std::list<SerialRequest_ptr>> SerialCommunicator::sendqueue;
std::map<int, std::list<SerialRequest_ptr>> SerialCommunicator::recvqueue;

/**
  A Serial communicator that implements the Comm interface for single core runs.
**/
INJECTION_COMM(VNVPACKAGENAME, serial) { return new SerialCommunicator(); }
