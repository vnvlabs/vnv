#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h> //hostent

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streaming/streamtemplate.h"

using namespace VnV::VNVPACKAGENAME::Engines::Streaming;
using nlohmann::json;

class TcpClient {
 private:
  int sock = -1 ;
  struct sockaddr_in server;

  bool conn(std::string address, int port) {
    // create socket if it is not already created
    if (sock == -1) {
      sock = socket(AF_INET, SOCK_STREAM, 0);
      if (sock == -1) {
        throw VnV::VnVExceptionBase("Could not create socket");
      }
    } 

    // setup address structure
    if (inet_addr(address.c_str()) == -1) {
      
      struct hostent* he;
      struct in_addr** addr_list;

      if ((he = gethostbyname(address.c_str())) == NULL) {
        // gethostbyname failed
        throw VnV::VnVExceptionBase("Failed to resove hostname");
      }

      // Cast the h_addr_list to in_addr , since h_addr_list also has the ip
      // address in long format only
      addr_list = (struct in_addr**)he->h_addr_list;
      for (int i = 0; addr_list[i] != NULL; i++) {
        server.sin_addr = *addr_list[i];
        break;
      }
    } else {
      server.sin_addr.s_addr = inet_addr(address.c_str());
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Connect to remote server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
      throw VnV::VnVExceptionBase("connect failed. Error");
    }
    return true;
  }

 public:
  TcpClient(std::string address, int port) {
    conn(address,port);
  };
  
  bool send_data(const std::string& data) {
    {
      std::ostringstream oss;
      oss << data.size() << ":" << data << ",";
      std::string s = oss.str();  

      // Send some data
      if ( send(sock, s.c_str(), strlen(s.c_str()), 0) < 0 ) {
         throw VnV::VnVExceptionBase("Send Failed");
      }
      return true;
    }
  }
};

class JsonSocketStream : public StreamWriter<json> {
  std::unique_ptr<TcpClient> client;
  
 public:
  virtual void initialize(json& config, bool readMode) override {
    if (readMode) {
       client = std::make_unique<TcpClient>(
         config["address"].get<std::string>(), 
         config["port"].get<int>()
      );
    } else {
      VnV::VnVExceptionBase("No read mode support for json_socket");
    }
  }

  virtual nlohmann::json getConfigurationSchema(bool readMode) override { return json::object(); };

  virtual void finalize(ICommunicator_ptr worldComm) override {
    // Close all the streams
    json j = json::object();
    j["DONE"] = "DONE";
    write(-1, j, -1);  
  }

  virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override {
    write(id, obj, -1);
  };

  virtual void write(long id, const json& obj, long jid) override {
    json j = json::object();
    j["stream"] = id;
    j["jid"] = jid;
    j["data"] = obj;
    client->send_data(j.dump());
    std::string s = j.dump();
  };

  virtual std::shared_ptr<IRootNode> parse(std::string file, long& id) {
    VnV_Error(VNVPACKAGENAME, "Http File Stream has no read option");
    return nullptr;
  }
};

INJECTION_ENGINE(VNVPACKAGENAME, json_socket) {
  return new StreamManager<json>(std::make_shared<JsonSocketStream>());
  ;
}
