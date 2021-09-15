#include <curl/curl.h>
#include <microhttpd.h>
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

namespace {

class Request {
 public:
  struct MHD_PostProcessor* pp;
  std::vector<char> streamId;
  std::vector<char> id;
  std::vector<char> data;

  ~Request() {
    MHD_destroy_post_processor(pp);
    pp = NULL;
  }

  long getStreamId() {
    std::string s(streamId.begin(), streamId.end());
    return std::atol(s.c_str());
  }
  long getId() {
    std::string s(id.begin(), id.end());
    return std::atol(s.c_str());
  }
  json getData() { return json::parse(data); }
};

MHD_Result set(std::vector<char>& data, uint64_t off, std::size_t size,
               const char* d);

MHD_Result post_iter(void* cls, enum MHD_ValueKind kind, const char* key,
                     const char* filename, const char* content_type,
                     const char* transfer_encoding, const char* data,
                     uint64_t off, std::size_t size);

MHD_Result answer_to_connection(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                std::size_t* upload_data_size, void** con_cls);

class JsonHttpStreamIterator : public VnV::StreamReader::JsonStreamIterator {
  int port = 0;
  struct MHD_Daemon* daemon;

 public:
  JsonHttpStreamIterator(int port_) : JsonStreamIterator(), port(port_){};

  bool start_stream_reader() override {
    if (daemon != NULL) {
      daemon =
          MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL,
                           &answer_to_connection, this, NULL, MHD_OPTION_END);
    }
    return daemon != NULL;
  }

  void stop_stream_reader() override {
    if (daemon != NULL) {
      MHD_stop_daemon(daemon);
      daemon == NULL;
    }
  }

  ~JsonHttpStreamIterator() { stop_stream_reader(); }
};

class JsonHttpStream : public StreamWriter<json> {
  CURL* curl;
  struct curl_slist* headers = NULL;
  std::string filestub;

 public:
  virtual void initialize(json& config, bool readMode) override {
    if (!readMode) {
      curl_global_init(CURL_GLOBAL_ALL);

      this->filestub = config["url"].get<std::string>();
      this->curl = curl_easy_init();
      if (!curl) {
        throw VnV::VnVExceptionBase("Could not set up curl");
      }
      headers = curl_slist_append(headers, "Accept: application/json");
      headers = curl_slist_append(headers, "Content-Type: application/json");
      headers = curl_slist_append(headers, "charset: utf-8");

      curl_easy_setopt(curl, CURLOPT_URL, filestub.c_str());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
    } else {
      throw VnV::VnVExceptionBase("No Read Mode for json_http");
    }
  }
  virtual nlohmann::json getConfigurationSchema(bool readMode) override {
    return json::object();
  };

  virtual void finalize(ICommunicator_ptr worldComm, long duration) override {
    // Close all the streams
    json j = json::object();
    j["DONE"] = "DONE";
    write(-1, j, -1);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  virtual void newComm(long id, const json& obj,
                       ICommunicator_ptr comm) override {
    write(id, obj, -1);
  };

  virtual void write(long id, const json& obj, long jid) override {
    json j = json::object();
    j["stream"] = id;
    j["jid"] = jid;
    j["data"] = obj;
    std::string s = j.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s.c_str());
    curl_easy_perform(curl);
  };

  static std::shared_ptr<IRootNode> parse(std::string file, long& id) {
    auto stream =
        std::make_shared<JsonHttpStreamIterator>(std::atoi(file.c_str()));

    return VnV::StreamReader::RootNodeWithThread::parse(id, stream);
  }
};

MHD_Result set(std::vector<char>& data, uint64_t off, std::size_t size,
               const char* d) {
  if (size + off > data.size()) {
    data.resize(size + off);
  }
  for (int i = 0; i < size; i++) {
    data[off + i] = data[i];
  }
  return MHD_YES;
}

MHD_Result post_iter(void* cls, enum MHD_ValueKind kind, const char* key,
                     const char* filename, const char* content_type,
                     const char* transfer_encoding, const char* data,
                     uint64_t off, std::size_t size) {
  Request* request = (Request*)cls;

  if (0 == strcmp("streamId", key)) {
    return set(request->streamId, off, size, data);
  } else if (0 == strcmp("jid", key)) {
    return set(request->id, off, size, data);
  } else if (0 == strcmp("data", key)) {
    return set(request->data, off, size, data);
  }
  return MHD_YES;
}

MHD_Result answer_to_connection(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                std::size_t* upload_data_size, void** con_cls) {
  const char* page = "OK";

  JsonHttpStreamIterator* iter = (JsonHttpStreamIterator*)(cls);
  Request* request = (Request*)(*con_cls);

  if (request == NULL) {
    request = new Request();
    *con_cls = request;
    if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
      request->pp =
          MHD_create_post_processor(connection, 1024, &post_iter, request);
      if (request->pp == NULL) {
        return MHD_NO; /* internal error */
      }
    }
    return MHD_YES;
  }

  if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
    /* evaluate POST data */
    MHD_post_process(request->pp, upload_data, *upload_data_size);
    if (0 != *upload_data_size) {
      *upload_data_size = 0;
      return MHD_YES;
    }

    bool found = false;
    for (auto& itt : iter->getInputStreams()) {
      if (itt->streamId() == request->getStreamId()) {
        found = true;
        itt->add(request->getId(), request->getData());
        break;
      }
    }
    if (!found) {
      auto s = std::make_shared<VnV::StreamReader::JsonSingleStreamIterator>(
          request->getStreamId());
      s->add(request->getId(), request->getData());
      iter->add(s);
    }
    delete request;

    struct MHD_Response* response;
    response = MHD_create_response_from_buffer(strlen(page), (void*)page,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }
  // Bad request.
  struct MHD_Response* response;
  response = MHD_create_response_from_buffer(strlen(page), (void*)page,
                                             MHD_RESPMEM_PERSISTENT);
  MHD_Result ret =
      MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
  MHD_destroy_response(response);
  return ret;
}

}  // namespace

INJECTION_ENGINE(VNVPACKAGENAME, json_http) {
  return new StreamManager<json>(std::make_shared<JsonHttpStream>());
}

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_http) {
  return JsonHttpStream::parse(filename, id);
}