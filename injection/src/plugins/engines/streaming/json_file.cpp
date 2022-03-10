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

  //What do we want to do--
  //We want to prefetch json.
  #define QUEUESIZE 100
  std::queue<json> jsonQueue;   
  std::atomic<bool> _writing = ATOMIC_VAR_INIT(false);
  std::atomic<bool> _fetching = ATOMIC_VAR_INIT(true);


  std::thread fetcher;


  void fetchThread () {
    while (_fetching.load()) {
      ifs.seekg(p);
      std::string currline;
      lockfile.lock();

      while (std::getline(ifs, currline)) {
        
        while (jsonQueue.size() > QUEUESIZE) {
            std::this_thread::yield();  // System can go do something else if it wants. 
        }
        
        json j = json::parse(currline);
        _writing.store(true, std::memory_order_relaxed);
        jsonQueue.push(json::parse(currline));
        _writing.store(false,  std::memory_order_relaxed);

        if (ifs.tellg() == -1) {
          p += currline.size();
        } else {
          p = ifs.tellg();
        }

      }
      ifs.clear();
    }
  }
  void launchThread() {
     fetcher = std::thread(&JsonFileIterator::fetchThread, this);
  }
  void killThread() {
    _fetching.store(false);
    if (fetcher.joinable()) {
      fetcher.join();
    }
  }

  void getLine_() {

    auto s = jsonQueue.size();
    if (s == 1) {
      while (_writing.load(std::memory_order_relaxed)){
        s = jsonQueue.size();
      }
    }
    if (s == 0 || (s == 1 && _writing.load(std::memory_order_relaxed)) ) {
      nextValue = STREAM_READER_NO_MORE_VALUES;
      nextCurr = json::object();
    } else {
      auto j = jsonQueue.front();
      jsonQueue.pop();
      nextCurr = j["object"];
      nextValue = j["id"].get<long>();
    }

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
      throw INJECTION_EXCEPTION("Could not open file %s", filename_.c_str());
    }
    launchThread();
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
    killThread();
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
  virtual void finalize(ICommunicator_ptr wcomm, long currentTime) override {
    for (auto& it : streams) {
      if (it.second.good()) {
        json j = json::object();
        j[JSD::node] = JSN::done;
        j[JSD::time] = currentTime;
        it.second << "{ \"id\": " << -1204 << ", \"object\" : " << j.dump() << "}" << std::endl;
      }
    }
    // Write a done file. (speeds up reading in static cases as we can stop waiting for new files. )
    std::ofstream(getFileName_(filestub,"__done__"));
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
    return VnV::DistUtils::join({stub, "__response__", std::to_string(jid) + "_" + std::to_string(id)}, 0777, true,
                                true);
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
        throw INJECTION_EXCEPTION_("Invalid Output file stream");
      }
    }
    throw INJECTION_EXCEPTION("Tried to write to a non-existent stream with id %ld", id);
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
  MultiFileStreamIterator(std::string fstub) : MultiStreamIterator<JsonFileIterator, json>(), filestub(fstub) {}

  virtual void respond(long id, long jid, const json& response) override {
    // Respond to a file request in a format that will be understood by rhe Json Stream that is
    // waiting for it.
    std::string s = JsonFileStream::getResponseFileName(filestub, id, jid);
    std::ofstream ofs(s + ".responding");
    ofs << response.dump();
    ofs.close();
    VnV::DistUtils::mv(s + ".responding", s + ".complete");
  }

  bool allstreamsread = false;
  void updateStreams() override {
    
    if (allstreamsread) return;
    
    try {

      std::vector<std::string> files = VnV::DistUtils::listFilesInDirectory(filestub);
      std::string ext = extension;
      for (auto& it : files) {
        if (loadedFiles.find(it) == loadedFiles.end()) {
          loadedFiles.insert(it);
          try {
            if (it.compare("__done__") == 0 ) {
               allstreamsread = true;
            }
            if (it.compare(".") == 0 || it.compare("..") == 0 || it.size() <= ext.size()) {
              continue;
            }
            if (it.substr(it.size() - ext.size()).compare(ext) != 0) {
              continue;
            }

            long id = std::atol(it.substr(0, it.size() - ext.size()).c_str());
            std::string fname = VnV::DistUtils::join({filestub, it}, 0777, false);
            add(std::make_shared<JsonFileIterator>(id, fname));

          } catch (std::exception& e) {
          }
        }
      }
    } catch (std::exception& e) {
      throw INJECTION_EXCEPTION_("Not A Directory");
    }
  }
};

}  // namespace

INJECTION_ENGINE(VNVPACKAGENAME, json_file, JsonFileStream::getSchema()) {
  return new StreamManager<json>(std::make_shared<JsonFileStream>());
}

INJECTION_ENGINE_READER(VNVPACKAGENAME, json_file, VnV::Nodes::dispathSchema()) {
  return engineReaderDispatch<MultiFileStreamIterator, json>(async, config,
                                                             std::make_shared<MultiFileStreamIterator>(filename));
}
