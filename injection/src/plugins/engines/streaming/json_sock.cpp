#include <arpa/inet.h>
#include <curl/curl.h>
#include <microhttpd.h>
#include <netdb.h>  //hostent
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <queue>
#include <thread>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streaming/streamreader.h"

using namespace VnV::VNVPACKAGENAME::Engines::Streaming;
using nlohmann::json;

#define VNV_SPEC -100
#define MAXLINE 1024
namespace {

class JsonSocketStreamIterator;

  // Convert client address into a string we can hash
  std::string ossclientaddr(struct sockaddr_in* addr) {
    std::ostringstream oss;
    oss << addr->sin_addr.s_addr << ":" << addr->sin_port << ":" << addr->sin_family;
    return oss.str();
  }

void sendUPD(int sockfd, const struct sockaddr* servaddr, socklen_t addrlen, const std::string& message) {
  std::ostringstream oss;
  oss << message.size() << ":" << message;
  std::string buff = oss.str();
  
  std::size_t sendPosition = 0;
  std::size_t messageLength = buff.size();
  char* m = &(buff[0]);

  while (messageLength) {
    int chunkSize = (messageLength<MAXLINE)?messageLength:MAXLINE; //std::min(messageLength,MAXLINE);
 
    chunkSize = sendto(sockfd, m + sendPosition, chunkSize, MSG_CONFIRM, servaddr, addrlen);
    if (chunkSize >= 0 ) {
        messageLength -= chunkSize;
       sendPosition += chunkSize;
    } else {
      throw VnV::VnVExceptionBase("Error sending %d", chunkSize);
    }

  }
}

class UDPServer {
  
  JsonSocketStreamIterator* iter;
  int sockfd;
  std::vector<char> buffer;
  struct sockaddr_in servaddr, cliaddr;

  // Map stream ids to client name.
  std::map<long, std::string> clientStreamMap;
  std::thread worker;
  unsigned int len, n;
  std::atomic_bool done = ATOMIC_VAR_INIT(false);

 public:
  
  UDPServer(JsonSocketStreamIterator* i, int port) : iter(i) {
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;  // IPv4
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }

    buffer.resize(MAXLINE);
    len = sizeof(cliaddr);  // len is value/resuslt
  }



  void copySockAddr(struct sockaddr_in* parent, struct sockaddr_in* child) {
    memcpy(&child, parent, sizeof(struct sockaddr_in));
  }
  std::map<std::string, std::pair<long, std::vector<char>>> messages;
  // Run the server;

  void stop() { done.store(true, std::memory_order_relaxed); }

  void run() { worker = std::thread(&UDPServer::runner, this); }

  void runner() {
     struct timeval tv;
     tv.tv_sec = 1;
     tv.tv_usec = 100000;
     if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
     }

    while (!done.load(std::memory_order_relaxed)) {
      
      int n = recvfrom(sockfd, &(buffer[0]), MAXLINE, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
      if (n < 0) continue;

      std::string c = ossclientaddr(&cliaddr);
      auto it = messages.find(c);
      std::vector<std::string> recv_messages;

      
      if (it == messages.end()) {
          long mcount = -1;
          std::vector<char> nmessage;
          processmessage((const struct sockaddr*)&cliaddr, len, c, buffer, n, mcount, nmessage);
          messages[c] = std::make_pair(mcount, nmessage);
      } else {
        std::string ss(buffer.begin(), buffer.begin() + n);
        processmessage((const struct sockaddr*)&cliaddr, len, c, buffer, n, it->second.first, it->second.second);
      }

    }
  }

  void processmessage(const struct sockaddr* sender, socklen_t len, std::string c, std::vector<char>& buffer, long recvCount,
                      long& messageCount, std::vector<char>& message) {
    
    std::string ss(buffer.begin(),buffer.begin()+recvCount);
    
    for (int i = 0; i < recvCount; i++) {
      if (messageCount < 0) {
        if (buffer[i] == ':') {
          
          std::string l(message.begin(), message.end());
          messageCount = std::atol(l.c_str());
          
          message.clear();
          message.reserve(messageCount);
        } else {
          message.push_back(buffer[i]);
        }
      } else {
        message.push_back(buffer[i]);
        if (message.size() == messageCount) {

          std::string s(message.begin(), message.end());
          parseMessage(sender, len, c, s);
          message.clear();
          messageCount = -1;
        }
      }
    }
    
  }

  void parseMessage(const struct sockaddr* sender, socklen_t sendlen, std::string client, std::string message);

  bool getResponse(long streamId, long jid, json& resp);

  void respondWithFetchData(const struct sockaddr* sender, socklen_t sendlena,  long streamId, long jid) {
    json resp = json::object();
    if (getResponse(streamId, jid, resp)) {
      sendUPD(sockfd, sender, sendlena, resp.dump());
    } else {
      sendUPD(sockfd, sender, sendlena,  "NO");
    }
  }
};

class UPDClient {
 public:
  bool connected = false;
  int sockfd;
  struct sockaddr_in servaddr;
  unsigned int n, len;

  UPDClient(std::string domain, int port) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      throw VnV::VnVExceptionBase("socket creation failed");
    }
    // Filling server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(domain.c_str());
    len = sizeof(servaddr);

  
  }

  void setTimeout(int seconds, int microseconds) {
     struct timeval tv;
      tv.tv_sec = seconds;
      tv.tv_usec = microseconds;
      if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
         perror("Error");
      }
  }

  bool hello() {
    if (!connected) {

      std::string s = "5:Hello";
      int a = sendto(sockfd, &(s[0]), s.size(), MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));

      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 100000;
      if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
         perror("Error");
      }



      std::vector<char> buff;
      buff.resize(100);

      setTimeout(1,10000);
      if (recvfrom(sockfd, &(buff[0]), 100, 0, (struct sockaddr*)&servaddr, &len) < 0) {
        connected = false;
      } else {
        connected = true;
      }
      setTimeout(0,0);
    
    }

    return connected;
  }

  void goodbye() { sendUPD(sockfd, (struct sockaddr*)&servaddr, len, "Goodbye"); }

  void send(json& j) { sendUPD(sockfd, (struct sockaddr*)&servaddr, len, j.dump()); }

  // Recieve a response from the server. !!!!!
  std::string recv(long streamid, long jid) {
    json j = json::object();
    j["stream"] = streamid;
    j["jid"] = jid;
    j["fetch"] = "FETCH";
    send(j);

    int r = 0;
    bool done = false;
    std::string ret = "";
    std::size_t recvMessCount = 0;

    // Used to recv messages from server.
    long messageLen = -1;
    std::vector<char> message;
    
    std::vector<char> buffer;
    buffer.reserve(MAXLINE);

    while (!done) {

      auto r = recvfrom(sockfd, &(buffer[0]), MAXLINE, MSG_WAITALL, (struct sockaddr*)&servaddr, &len);

      for (int i = 0; i < r; i++) {
        if (messageLen < 0) {
          if (buffer[i] == ':') {
            std::string l(message.begin(), message.end());
            messageLen = std::atol(l.data());
            message.reserve(messageLen);
            message.clear();
          } else {
            message.push_back(buffer[i]);
          }
        } else {
          message.push_back(buffer[i]);
          if (message.size() == messageLen) {
            std::string s(message.begin(), message.end());            
            return s;
            // We can Ignore garbage after recv due to design.
          }
        }
      }
    }
    
    std::string s(message.begin(), message.end());
    return s;
  }

  ~UPDClient() { close(sockfd); }
};

class JsonSocketStreamIterator : public VnV::StreamReader::JsonPortStreamIterator {
  std::string rdir;
  std::shared_ptr<UDPServer> server = nullptr;

 public:
  JsonSocketStreamIterator(std::string port) : VnV::StreamReader::JsonPortStreamIterator(port){};

  bool start_stream_reader() override {
    if (server == nullptr) {
      server.reset(new UDPServer(this, port));
      server->run();
    }
    return server != nullptr;
  }

  void stop_stream_reader() override {
    if (server != nullptr) {
      server->stop();
      server.reset();
    }
  }

  ~JsonSocketStreamIterator() { stop_stream_reader(); }
};

class JsonSocketStream : public StreamWriter<json> {
  std::shared_ptr<UPDClient> client;
  std::string filestub;
  std::map<long, std::map<long, json>> fetchs;

 public:
  virtual void initialize(json& config, bool readMode) override {
    this->filestub = config["filename"].get<std::string>();
    int port = config.value("port", 5004);

    client.reset(new UPDClient(filestub, port));

    while (!client->hello()) {
      std::cout << "Trying to connect...." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    std::cout << "Connection Successfull --- Lets Gooooooo!" << std::endl;
  }

  virtual bool supportsFetch() override { return true; }

  virtual bool fetch(long id, long jid, json& response) override {
    std::string s = client->recv(id, jid);
    if (s.compare("NO") == 0) {
      return false;
    } else {
      response = json::parse(s);
      return true;
    }
  }

  virtual nlohmann::json getConfigurationSchema(bool readMode) override { return json::object(); };

  virtual void finalize(ICommunicator_ptr worldComm, long duration) override {
    // Close all the streams
    client->goodbye();
    client.reset();
  }

  virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override { write(id, obj, -1); };

  virtual void write(long id, const json& obj, long jid) override {
    json j = json::object();
    j["stream"] = id;
    j["jid"] = jid;
    j["data"] = obj;
    client->send(j);
  };
};

bool UDPServer::getResponse(long streamId, long jid, json& resp) {
   return iter->getResponse(streamId, jid, resp);
}


void UDPServer::parseMessage(const struct sockaddr* sender, socklen_t lena,  std::string client, std::string data) {
  if (data.compare("Hello") == 0) {
    sendUPD(sockfd, sender, len,  "Hi");
  } else if (data.compare("Goodbye") == 0) {
    iter->done();
  } else {
    
    try {
      json j = json::parse(data);
      long stream = j["stream"].get<long>();
      long jid = j["jid"].get<long>();

      if (j.contains("fetch")) {
        respondWithFetchData(sender, lena, stream, jid);
        return;
      }

      json& data = j["data"];

      clientStreamMap[stream] = client;

      bool found = false;
      for (auto& itt : iter->getInputStreams()) {
        if (itt->streamId() == stream) {
          found = true;
          itt->add(jid, data);
          break;
        }
      }
      if (!found) {
        auto s = std::make_shared<VnV::StreamReader::JsonSingleStreamIterator>(stream);
        s->add(jid, data);
        iter->add(s);
      }
    } catch (...) {
      throw VnV::VnVExceptionBase("COULD NOT PARSE %s ", data );
    }
  }
}

}  // namespace

INJECTION_ENGINE(VNVPACKAGENAME, json_socket) { return new StreamManager<json>(std::make_shared<JsonSocketStream>()); }

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_socket) {
  auto stream = std::make_shared<JsonSocketStreamIterator>(filename.c_str());
  return RootNodeWithThread<JsonSocketStreamIterator, json>::parse(
      id, stream, std::make_shared<VnV::StreamReader::JsonStreamVisitor>());
}