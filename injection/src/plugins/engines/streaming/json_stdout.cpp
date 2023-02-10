#include <fstream>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "streaming/streamtemplate.h"

using namespace VnV::Nodes;
using nlohmann::json;

class JsonTerminalStream : public StreamWriter<json> {
 public:
  virtual void initialize(json& config) override {}

  virtual void finalize(ICommunicator_ptr worldComm, long currentTime) override {
    if (worldComm->Rank() == 0) {
      std::cout << "END Time in ms since epoch: " << currentTime << std::endl;
    }
  }

  virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override { write(id, obj, -1); };

  json getRunInfo() override { return json::object(); }

  virtual void write(long id, const json& obj, long jid) override {
    std::cout << "\n\n[STREAM " << id << "]\n" << obj.dump(3) << "\n\n";
    std::cout.flush();
  };

  virtual std::shared_ptr<IRootNode> parse(std::string file, long& id) {
    VnV_Error(VNVPACKAGENAME, "Http File Stream has no read option");
    return nullptr;
  }
};

INJECTION_ENGINE(VNVPACKAGENAME, json_stdout, "{}") {
  return new StreamManager<json>(std::make_shared<JsonTerminalStream>());
}

INJECTION_ENGINE(VNVPACKAGENAME, json, "{}") {
  VnV_Warn(VNVPACKAGENAME,
           "The \"json\" engine is DEPRECIATED, please use one of "
           "\"json_{stdout,socket,http,file}\" instead");
  return new StreamManager<json>(std::make_shared<JsonTerminalStream>());
}
