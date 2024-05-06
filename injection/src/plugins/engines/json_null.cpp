
#include "interfaces/IOutputEngine.h"


namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

using nlohmann::json;
class NullManager : public OutputEngineManager {
 public:
  NullManager() {}

  virtual ~NullManager() {}

  bool Fetch(std::string message, const json& schema, long timeoutInSeconds, json& response) override { return false; }

#define LTypes X(double&) X(long long&) X(bool&) X(std::string&) X(json&) X(IDataType_ptr)
#define X(typea) \
  void Put(std::string variableName, const typea value, const MetaData& m) override {}
  LTypes
#undef X
#undef LTypes

      void
      PutGlobalArray(long long dtype, std::string variableName, IDataType_vec data, std::vector<int> gsizes,
                     std::vector<int> sizes, std::vector<int> offset, const MetaData& m)  override {
  }
  void Log(ICommunicator_ptr logcomm, const char* package, int stage, std::string level, std::string message) override {}

  void file(VnV::ICommunicator_ptr, std::string, std::string, bool, std::string, std::string, std::string) override {}

  void finalize(ICommunicator_ptr worldComm, long duration) override {}

  void setFromJson(ICommunicator_ptr worldComm, json& config) override {}
  void sendInfoNode(ICommunicator_ptr worldComm, const json& fullJson, const json& prov, std::string workflowName, std::string workflowJob ) override {}

  void write_stdout(ICommunicator_ptr comm, const std::string& out) override {}

  void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageVal) override {}

  void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string id,
                                     InjectionPointType type, std::string stageVal, std::string filename,
                                     int line) override {}
  void testStartedCallBack(std::string packageName, std::string testName, bool internal, long uuid) override {}

  void testFinishedCallBack(bool result_) override {}

  void actionStartedCallBack(ICommunicator_ptr comm, std::string package, std::string name,
                             std::string stage) override{};

  virtual void actionEndedCallBack(std::string stage) override{};

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string unitTestName) override {}

  virtual void packageOptionsStartedCallBack(ICommunicator_ptr comm, std::string packageName) override {}

  virtual void packageOptionsEndedCallBack(std::string packageName) override {}

  virtual void initializationStartedCallBack(ICommunicator_ptr comm, std::string packageName)override {};
  virtual void initializationEndedCallBack(std::string packageName)override {};

  void unitTestFinishedCallBack(IUnitTest* tester)override {}

  json getRunInfo() override { return json::object(); }

  // IInternalOutputEngine interface
  std::string print() override { return ""; }
};

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV

INJECTION_ENGINE(VNVPACKAGENAME, null, "{}") { return new NullManager(); }
