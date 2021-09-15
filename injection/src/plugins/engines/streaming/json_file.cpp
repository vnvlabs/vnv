#include <unistd.h>

#include <chrono>
#include <fstream>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streaming/streamtemplate.h"

using namespace VnV::VNVPACKAGENAME::Engines::Streaming;
using nlohmann::json;

#define STREAM_READER_NO_MORE_VALUES -19999

// A static file iterator.
class JsonFileIterator : public Iterator<json> {
  long sId;
  std::ifstream ifs;
  std::streamoff p = 0;

  json nextCurr;
  long nextValue = -1;

  std::string currJson;

  void getLine_() {
    std::string currline;
    ifs.seekg(p);
    if (std::getline(ifs, currline)) {
      json t = json::parse(currline);
      nextCurr = t["object"];
      nextValue = t["id"].get<long>();

      if (ifs.tellg() == -1) {
        p += currline.size();
      } else {
        p = ifs.tellg();
      }

    } else {
      nextValue = STREAM_READER_NO_MORE_VALUES;
      nextCurr = json::object();
      ifs.clear();  // Clear the stream so we can try and read again.
    }
  }

  void getLine(json& current, long& currentValue) override {
    current = nextCurr;
    currentValue = nextValue;
    getLine_();
    return;
  }

 public:
  JsonFileIterator(long streamId_, std::string filename_)
      : sId(streamId_), ifs(filename_) {
    if (!ifs.good()) {
      throw VnV::VnVExceptionBase("Could not open file %s", filename_.c_str());
    }
    getLine_();
  }

  bool hasNext() const override {
    return nextValue != STREAM_READER_NO_MORE_VALUES;
  }

  long streamId() const override { return sId; }

  ~JsonFileIterator() { ifs.close(); }
};

class JsonFileStream : public FileStream<JsonFileIterator, json> {
  std::map<long, std::ofstream> streams;
  static std::size_t INIDCOUNT;
  std::string response_stub = "";

 public:
  virtual void finalize(ICommunicator_ptr wcomm, long duration) override {
    for (auto& it : streams) {
      if (it.second.good()) {
        json j = json::object();
        j[JSD::node] = JSN::done;
        j[JSD::duration] = VnV::RunTime::instance().duration();
        it.second << "{ \"id\": " << -1204 << ", \"object\" : " << j.dump()
                  << "}" << std::endl;
      }
    }
  }

  std::string createResponseDirectory() {
    if (response_stub.length() == 0) {
      response_stub =
          VnV::DistUtils::join({filestub, "__response__"}, 0777, true);
    }  // generate a filename filestup/__response__/<id>_<comm>
    return response_stub;
  }

  virtual void newComm(long id, const json& obj,
                       ICommunicator_ptr comm) override {
    if (streams.find(id) == streams.end()) {
      std::ofstream off(getFileName(id));
      streams.insert(std::make_pair(id, std::move(off)));
      write(id, obj, -1);
    }
  };

  virtual bool fetch(long id, const json& schema, long timeoutInSeconds,
                     json& response) {
    // Get a unique id for this request.
    std::string c = std::to_string(++INIDCOUNT);
    std::string fname = VnV::DistUtils::join(
        {createResponseDirectory(), c + "_" + std::to_string(id)}, 0777, false);

    // Open a file called <fname>.writing.
    std::ofstream f(fname + ".writing");

    // Write the schema provided and the expiry time to the file.
    long expiry = std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count() +
                  timeoutInSeconds;
    json j = json::object();
    j["schema"] = schema;
    j["expires"] = expiry;
    f << j.dump(4);
    f.close();

    // Rename the file the <fname>.pending. This makes sure the reader does not
    // see the file until we are done writing to it.
    VnV::DistUtils::mv(fname + ".writing", fname + ".pending");

    // Loop until timeout
    while (std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
               .count() < expiry) {
      // Look for a file called fname.complete.
      std::ifstream ifs(fname + ".complete");
      if (ifs.good()) {
        // If we found it then parse the contents into result.
        response = json::parse(ifs);
        return true;
      }

      // Go to sleep for a second to avoid spamming the filesystem
      sleep(1);
    }

    // We timed out so continue.
    return false;
  }

  virtual void write(long id, const json& obj, long jid) override {
    auto it = streams.find(id);

    if (it != streams.end()) {
      if (it->second.good()) {
        // Writing to a closed stream does NOT throw an exception (by default).
        // So, we check for "good" first/
        it->second << "{ \"id\": " << jid << ", \"object\" : " << obj.dump()
                   << "}" << std::endl;
        it->second.flush();
        return;

      } else {
        throw VnV::VnVExceptionBase("Invalid Output file stream");
      }
    }
    throw VnV::VnVExceptionBase(
        "Tried to write to a non-existent stream with id %ld", id);
  };

  virtual ~JsonFileStream() {
    for (auto& it : streams) {
      it.second.close();
    }
  }
};

std::size_t JsonFileStream::INIDCOUNT = 0;

INJECTION_ENGINE(VNVPACKAGENAME, json_file) {
  return new StreamManager<json>(std::make_shared<JsonFileStream>());
}

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_file) {
  return JsonFileStream::parse(filename, id);
}