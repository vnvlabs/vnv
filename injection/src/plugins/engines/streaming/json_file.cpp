#include <fstream>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streaming/streamtemplate.h"

using namespace VnV::VNVPACKAGENAME::Engines::Streaming;
using nlohmann::json;

// A static file iterator.
class JsonFileIterator : public Iterator<json> {
  long sId;
  std::ifstream ifs;

  bool more;
  long currId;
  std::string currJson;

  void getLine(json& current, long& currentValue) override {
    std::string currline;
    if (std::getline(ifs, currline)) {
      json t = json::parse(currline);
      current = t["object"];
      currentValue = t["id"].get<long>();
      more = true;
    } else {
      current = json::object();
      currentValue = -1;
      more = false;
    }
  }

 public:
  JsonFileIterator(long streamId_, std::string filename_)
      : sId(streamId_), ifs(filename_) {}

  bool hasNext() const override { return more; }

  bool isDone() const override { return more; }

  long streamId() const override { return sId; }

  ~JsonFileIterator() { ifs.close(); }
};

class JsonFileStream : public FileStream<JsonFileIterator,json> {
  
  std::map<long, std::ofstream> streams;

 public:

  virtual void finalize(ICommunicator_ptr wcomm) override {
    if (wcomm->Rank() == 0) {
      // Close all the streams
      for (auto& it : streams) {
        it.second.close();
      }
    }
  }

  virtual void newComm(long id, const json& obj,
                       ICommunicator_ptr comm) override {
    if (streams.find(id) == streams.end()) {
      std::ofstream off(getFileName(id));
      streams.insert(std::make_pair(id, std::move(off)));
      write(id, obj, -1);
    }
  };

  virtual void write(long id, const json& obj, long jid) override {
    auto it = streams.find(id);
    if (it != streams.end()) {
      it->second << "\n{ \"id\": " << jid << ", \"object\" : " << obj.dump() << "}";
    }
  };
};

INJECTION_ENGINE(VNVPACKAGENAME, json_file) { 
    return new StreamManager<json>(std::make_shared<JsonFileStream>());
}
