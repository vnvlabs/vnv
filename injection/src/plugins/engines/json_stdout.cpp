#include <fstream>

#include "shared/DistUtils.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/engines/streamtemplate.h"

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

  std::string getFileStub() override {return "stdout";}

  virtual void write(long id, const json& obj, long jid) override {
    std::cout << "\n\n[STREAM " << id << "]\n" << obj.dump(3) << "\n\n";
    std::cout.flush();
  };

};

INJECTION_ENGINE(VNVPACKAGENAME, stdout, "{}") {
  return new StreamManager<json>(std::make_shared<JsonTerminalStream>());
}
