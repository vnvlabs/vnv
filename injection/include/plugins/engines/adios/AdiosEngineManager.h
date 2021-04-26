#ifndef ADIOSENGINEMANAGER_H
#define ADIOSENGINEMANAGER_H

#include <map>
#include <sstream>
#include <string>

#include "adios2.h"
#include "base/CommMapper.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"
#include "plugins/engines/adios/AdiosEngineImpl.h"
using nlohmann::json;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class AdiosEngineManager : public OutputEngineManager {
  static int ADIOS_ENGINE_MANAGER_VERSION;

  long id = 0;

  CommMapper commMapper;

  adios2::ADIOS* adios; /**< @todo  */
  adios2::IO bpWriter;  /**< @todo  */
  adios2::Engine worldEngine;

  int worldRank = -1;

  std::map<long, std::shared_ptr<AdiosEngineImpl>> routes;
  std::shared_ptr<AdiosEngineImpl> curr;

  std::string outfile = "./vnv-adios.out";

  unsigned int outputFile; /**< @todo  */

 public:
  AdiosEngineManager();

  ~AdiosEngineManager() override;

  void setComm(ICommunicator_ptr comm);
  long getNextId(ICommunicator_ptr comm);

  std::string getFileName(std::vector<std::string> fname);

  void Put(std::string variableName, const double& value,
           const MetaData& m) override;

  void Put(std::string variableName, const long long& value,
           const MetaData& m) override;

  void Put(std::string variableName, const bool& value,
           const MetaData& m) override;

  void Put(std::string variableName, const json& value,
           const MetaData& m) override;

  void Put(std::string variableName, const std::string& value,
           const MetaData& m) override;

  void PutGlobalArray(long long dtype, std::string variableName,
                      IDataType_vec data, std::vector<int> gsizes,
                      std::vector<int> sizes, std::vector<int> offset,
                      const MetaData& m) override;

  void Log(ICommunicator_ptr /** comm **/, const char* package, int stage,
           std::string level, std::string message) override;

  void finalize(ICommunicator_ptr worldComm) override;

  void setFromJson(json& config) override;

  void injectionPointEndedCallBack(std::string id, InjectionPointType type_,
                                   std::string stageId) override;

  void injectionPointStartedCallBack(ICommunicator_ptr comm ,
                                     std::string packageName, std::string id,
                                     InjectionPointType type_,
                                     std::string stageId) override;

  void testStartedCallBack(std::string packageName, std::string testName,
                           bool internal) override;

  void testFinishedCallBack(bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr comm ,
                               std::string packageName,
                               std::string unitTestName) override;
  void unitTestFinishedCallBack(IUnitTest* tester) override;

  void dataTypeStartedCallBack(std::string variableName, long long dtype,
                               const MetaData& m) override;
  void dataTypeEndedCallBack(std::string variableName) override;
  json getConfigurationSchema() override;

  std::string print() override;

  Nodes::IRootNode* readFromFile(std::string /**filename**/,
                                 long& idCounter) override;

  void commsMapSetToMap(CommWrap_ptr ptr, std::map<long, CommWrap_ptr>& comms) {
    auto it = comms.find(ptr->id);
    if (it == comms.end()) {
      comms.insert(std::make_pair(ptr->id, ptr));
      for (auto ch : ptr->children) {
        commsMapSetToMap(ch.second, comms);
      }
    }
  }
};

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // ADIOSENGINEMANAGER_H
