#include <curl/curl.h>

#include <fstream>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streaming/streamtemplate.h"
using namespace VnV::VNVPACKAGENAME::Engines::Streaming;
using nlohmann::json;

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

  virtual void finalize(ICommunicator_ptr worldComm) override {
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

  virtual std::shared_ptr<IRootNode> parse(std::string file, long& id) {
    VnV_Error(VNVPACKAGENAME, "Http File Stream has no read option");
    return nullptr;
  }
};

INJECTION_ENGINE(VNVPACKAGENAME, json_http) {
  return new StreamManager<json>(std::make_shared<JsonHttpStream>());
  ;
}
