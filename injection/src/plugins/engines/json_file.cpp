#include <unistd.h>

#include <chrono>
#include <fstream>
#include <queue>

#include "shared/DistUtils.h"
#include "shared/Utilities.h"
#include "shared/constants.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streamtemplate.h"
using nlohmann::json;
using namespace VnV::Nodes;
using namespace VnV::Constants;


namespace {

class JsonFileStream : public FileStream<json> {
  std::map<long, std::ofstream> streams;
  std::map<long, VnV::DistUtils::LockFile> lockfiles;

  std::string response_stub = "";

 public:
  virtual void finalize(ICommunicator_ptr wcomm, long currentTime) override {
    for (auto& it : streams) {
      if (it.second.good()) {
        json j = json::object();
        j[JSD::node] = JSN::done;
        j[JSD::time] = currentTime;
        it.second << "[" << -1204 << "," << j.dump() << "]" << std::endl;
      }
    }
    // Write a done file. (speeds up reading in static cases as we can stop waiting for new files. )
    std::ofstream(getFileName_(filestub, "__done__"));
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
    return VnV::DistUtils::join({stub, "__response__", std::to_string(jid) + "_" + std::to_string(id)}, 0777, true, true);
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
        it->second << "[" << jid << "," << obj.dump() << "]" << std::endl;
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

}  // namespace

INJECTION_ENGINE(VNVPACKAGENAME, file, JsonFileStream::getSchema()) {
  return new StreamManager<json>(std::make_shared<JsonFileStream>());
}
