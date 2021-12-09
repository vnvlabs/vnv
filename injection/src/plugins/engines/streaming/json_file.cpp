#include <unistd.h>

#include <chrono>
#include <fstream>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "interfaces/IOutputEngine.h"
#include "streaming/dispatch.h"

using namespace VnV::Nodes;
using nlohmann::json;

#define STREAM_READER_NO_MORE_VALUES -19999

namespace {

 // A static file iterator.
  class JsonFileIterator : public Iterator<json> {
    long sId;
    std::ifstream ifs;
    std::streamoff p = 0;
    VnV::DistUtils::LockFile lockfile;

    json nextCurr;
    long nextValue = -1;

    std::string currJson;

    void getLine_() {
      lockfile.lock();

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

      lockfile.unlock();
    }

    void getLine(json& current, long& currentValue) override {
      current = nextCurr;
      currentValue = nextValue;
      getLine_();
      return;
    }

   public:
    JsonFileIterator(long streamId_, std::string filename_) : sId(streamId_), ifs(filename_), lockfile(filename_) {
      if (!ifs.good()) {
        throw VnV::VnVExceptionBase("Could not open file %s", filename_.c_str());
      }
      getLine_();
    }

    bool hasNext() override {
      if (nextValue == STREAM_READER_NO_MORE_VALUES) {
        getLine_();
      }
      return nextValue != STREAM_READER_NO_MORE_VALUES;
    }

    long streamId() const override { return sId; }

    ~JsonFileIterator() {
      ifs.close();
      lockfile.close();
    }
  };

 class JsonFileStream : public FileStream<JsonFileIterator, json> {
    std::map<long, std::ofstream> streams;
    std::map<long, VnV::DistUtils::LockFile> lockfiles;

    static std::size_t INIDCOUNT;
    std::string response_stub = "";

   public:
    virtual void finalize(ICommunicator_ptr wcomm, long duration) override {
      for (auto& it : streams) {
        if (it.second.good()) {
          json j = json::object();
          j[JSD::node] = JSN::done;
          j[JSD::duration] = VnV::RunTime::instance().duration();
          it.second << "{ \"id\": " << -1204 << ", \"object\" : " << j.dump() << "}" << std::endl;
        }
      }
    }

    virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override {
      if (streams.find(id) == streams.end()) {
        std::ofstream off(getFileName(id));
        VnV::DistUtils::LockFile lock(getFileName(id));
        streams.insert(std::make_pair(id, std::move(off)));
        lockfiles.insert(std::make_pair(id, std::move(lock)));
        write(id, obj, -1);
      }
    };

    virtual bool supportsFetch() override { return true; }

    static std::string getResponseFileName(std::string stub, long id, long jid) {
      return VnV::DistUtils::join({stub, "__response__", std::to_string(jid) + "_" + std::to_string(id)}, 0777, true);
    }

    virtual bool fetch(long id, long jid, json& response) override {
      std::string s = getResponseFileName(filestub, id, jid);
      std::ifstream ifs(s + ".complete");
      if (ifs.good()) {
        response = json::parse(ifs);
        return true;
      }
      return false;
    }

    virtual void write(long id, const json& obj, long jid) override {
      auto it = streams.find(id);
      auto lo = lockfiles.find(id);

      if (it != streams.end()) {
        if (it->second.good()) {
          // Writing to a closed stream does NOT throw an exception (by default).
          // So, we check for "good" first/
          lo->second.lock();
          it->second << "{ \"id\": " << jid << ", \"object\" : " << obj.dump() << "}" << std::endl;
          it->second.flush();
          lo->second.unlock();
          return;

        } else {
          throw VnV::VnVExceptionBase("Invalid Output file stream");
        }
      }
      throw VnV::VnVExceptionBase("Tried to write to a non-existent stream with id %ld", id);
    };

    virtual ~JsonFileStream() {
      for (auto& it : streams) {
        it.second.close();
      }
      for (auto& it : lockfiles) {
        it.second.close();
      }
    }
  };

 std::size_t JsonFileStream::INIDCOUNT = 0;

class MultiFileStreamIterator : public MultiStreamIterator<JsonFileIterator, json> {
    std::set<std::string> loadedFiles;
    std::string filestub;
    std::string response_stub = "";
    
   public:
    MultiFileStreamIterator(std::string fstub) : MultiStreamIterator<JsonFileIterator, json>(), filestub(fstub) {
      bool changed = false;
      while (VnV::DistUtils::fileExists(filestub)) {
        filestub = fstub + VnV::TimeUtils::timestamp();
        changed = true;
      }

      if (changed) {
        VnV_Warn(VNVPACKAGENAME, "Output will be written to %s because %s already exists", filestub.c_str(),
                 fstub.c_str());
      }
    }

    virtual void respond(long id, long jid, const json& response) override {
      // Respond to a file request in a format that will be understood by rhe Json Stream that is
      // waiting for it.
      std::string s = JsonFileStream::getResponseFileName(filestub, id, jid);
      std::ofstream ofs(s + ".responding");
      ofs << response.dump();
      ofs.close();
      VnV::DistUtils::mv(s + ".responding", s + ".complete");
    }

    void updateStreams() override {
      std::vector<std::string> files = VnV::DistUtils::listFilesInDirectory(filestub);
      for (auto& it : files) {
        if (loadedFiles.find(it) == loadedFiles.end()) {
          loadedFiles.insert(it);
          std::size_t dot = it.find_first_of(".");
          try {
            if (it.substr(dot).compare(extension) == 0) {
              long id = std::atol(it.substr(0, dot).c_str());
              std::string fname = VnV::DistUtils::join({filestub, it}, 0777, false);
              add(std::make_shared<JsonFileIterator>(id, fname));
            }
          } catch (...) {
          }
        }
      }
    }
  };

}

INJECTION_ENGINE(VNVPACKAGENAME, json_file) { 
  return new StreamManager<json>(std::make_shared<JsonFileStream>());
}
 

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_file) {
  return engineReaderDispatch<MultiFileStreamIterator, json>(async, config, std::make_shared<MultiFileStreamIterator>(filename));
}

