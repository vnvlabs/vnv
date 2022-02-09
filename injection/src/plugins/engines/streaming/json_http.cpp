#include <microhttpd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <mutex>
#include <queue>
#include <thread>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "streaming/curl.h"
#include "streaming/dispatch.h"

using namespace VnV::Nodes;
using nlohmann::json;

#define VNV_SPEC -100

namespace {

#if WITH_MHD_RESULT == 1
using MHD_Result = int;
#endif

class Request {
 public:
  std::size_t off = 0;
  std::vector<char> data;

  ~Request() {}

  json getData() { return json::parse(data); }

  bool hello() {
    if (data.size() == 5) {
      std::string s(data.begin(), data.end());
      return s.compare("Hello") == 0;
    }
    return false;
  }
  bool goodbye() {
    if (data.size() == 7) {
      std::string s(data.begin(), data.end());
      return s.compare("Goodbye") == 0;
    }
    return false;
  }

  void add(const char* newdata, std::size_t size) {
    if ((off + size) > data.size()) {
      data.resize(size + off);
    }
    for (int i = 0; i < size; i++) {
      data[off + i] = newdata[i];
    }
    off += size;
  }
};

MHD_Result set(std::vector<char>& data, uint64_t off, std::size_t size, const char* d);

MHD_Result post_iter(void* cls, enum MHD_ValueKind kind, const char* key, const char* filename,
                     const char* content_type, const char* transfer_encoding, const char* data, uint64_t off,
                     std::size_t size);

MHD_Result answer_to_connection(void* cls, struct MHD_Connection* connection, const char* url, const char* method,
                                const char* version, const char* upload_data, std::size_t* upload_data_size,
                                void** con_cls);

class JsonHttpStreamIterator : public JsonPortStreamIterator {
  struct MHD_Daemon* daemon = NULL;

 public:
  JsonHttpStreamIterator(std::string p, const json& config) : JsonPortStreamIterator(p, config){};

  bool start_stream_reader() override {
    if (daemon == NULL) {
      daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL, &answer_to_connection, this, NULL,
                                MHD_OPTION_END);
    }
    return daemon != NULL;
  }

  void stop_stream_reader() override {
    std::cout << "SHUTTING DOWN THE DAEMON" << std::endl;
    if (daemon != NULL) {
      MHD_stop_daemon(daemon);
      daemon == NULL;
    }
  }

  ~JsonHttpStreamIterator() { stop_stream_reader(); }
};

size_t writefunc(void* ptr, size_t size, size_t nmemb, std::string* s) {
  s->append(static_cast<char*>(ptr), size * nmemb);
  return size * nmemb;
}

class JsonHttpStream : public PortStreamWriter<json> {
  VnV::Curl::CurlWrapper& curl;
  std::string filestub;
  std::string rr;

 public:
  JsonHttpStream() : curl(VnV::Curl::CurlWrapper::instance()) {}

  virtual void initialize(json& config) override {
    filestub = PortStreamWriter<json>::init("json_http", config);

    curl.setUrl(filestub);
    curl.setPostFields("Hello");

    while (true) {
      std::cout << "Trying to connect.... to " << filestub << std::endl;

      curl.send();

      if (curl.getCode() == 200) {
        std::cout << "Connection Successfull --- Lets Gooooooo!" << std::endl;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
  }

  virtual void finalize(ICommunicator_ptr worldComm, long currentTime) override {
    // Close all the streams
    std::string hello = "Goodbye";
    curl.setPostFields("Goodbye");
    curl.send();
  }

  virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override { write(id, obj, -1); };

  virtual void write(long id, const json& obj, long jid) override {
    json j = json::object();
    j["stream"] = id;
    j["jid"] = jid;
    j["data"] = obj;
    std::string s = j.dump();
    curl.setPostFields(j.dump());
    curl.send();
  };

  virtual bool supportsFetch() override { return true; }

  virtual bool fetch(long id, long jid, json& response) override {
    json j = json::object();
    j["stream"] = id;
    j["jid"] = jid;
    j["fetch"] = "FETCH";
    std::string s = j.dump();

    curl.clearData();
    curl.setPostFields(s);
    curl.send();

    if (curl.getCode() == 200) {
      response = json::parse(curl.getData());
      return true;
    } else {
      return false;
    }
  }
};

MHD_Result set(std::vector<char>& data, uint64_t off, std::size_t size, const char* d) {
  if (size + off > data.size()) {
    data.resize(size + off);
  }
  for (int i = 0; i < size; i++) {
    data[off + i] = data[i];
  }
  return MHD_YES;
}

MHD_Result answer_to_connection(void* cls, struct MHD_Connection* connection, const char* url, const char* method,
                                const char* version, const char* upload_data, std::size_t* upload_data_size,
                                void** con_cls) {
  const char* page = "OK";
  const char* err = "BAD";
  char* user;
  char* pass;

  pass = NULL;
  user = MHD_basic_auth_get_username_password(connection, &pass);

  JsonHttpStreamIterator* iter = (JsonHttpStreamIterator*)(cls);
  if (!iter->authorize(user, pass)) {
    // Respond as we are done.
    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, response);
    MHD_free(user);
    MHD_free(pass);
    MHD_destroy_response(response);
    return ret;
  }

  MHD_free(user);
  MHD_free(pass);

  Request* request = (Request*)(*con_cls);

  if (request == NULL) {
    request = new Request();
    *con_cls = request;
    return MHD_YES;
  }

  // If we have data available, process it, then return yes
  if (0 != *upload_data_size) {
    request->add(upload_data, *upload_data_size);
    *upload_data_size = 0;
    return MHD_YES;
  }

  if (request->hello()) {
    // Respond as we are done.
    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  } else if (request->goodbye()) {
    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    iter->done();
    return ret;
  }

  try {
    json j = request->getData();

    long stream = j["stream"].get<long>();
    long jid = j["jid"].get<long>();

    if (j.contains("fetch")) {
      struct MHD_Response* response;
      MHD_Result ret;

      json j = json::object();
      if (iter->getResponse(stream, jid, j)) {
        std::string resp = j.dump();
        response = MHD_create_response_from_buffer(resp.size(), (void*)resp.c_str(), MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      } else {
        response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
      }
      MHD_destroy_response(response);
      return ret;
    }

    json& data = j["data"];
    bool found = false;
    for (auto& itt : iter->getInputStreams()) {
      if (itt->streamId() == stream) {
        found = true;
        itt->add(jid, data);
        break;
      }
    }
    if (!found) {
      auto s = std::make_shared<JsonSingleStreamIterator>(stream);
      s->add(jid, data);
      iter->add(s);
    }
    delete request;

    // Respond as we are done.
    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;

  } catch (std::exception &e) {
    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(err), (void*)err, MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
    MHD_destroy_response(response);
    return ret;
  }
}

}  // namespace

INJECTION_ENGINE(VNVPACKAGENAME, json_http, JsonHttpStream::getSchema()) {
  return new StreamManager<json>(std::make_shared<JsonHttpStream>());
}

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_http, JsonHttpStreamIterator::getSchema(VnV::Nodes::dispathSchema())) {
  return engineReaderDispatch<JsonHttpStreamIterator, json>(
      async, config, std::make_shared<JsonHttpStreamIterator>(filename, config), false);
}
