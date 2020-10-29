#ifndef ADIOSENGINEMANAGER_H
#define ADIOSENGINEMANAGER_H

#include <map>
#include <sstream>
#include <string>

#include "adios2.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class AdiosEngineManager : public OutputEngineManager {
 public:
  adios2::ADIOS* adios;                     /**< @todo  */
  adios2::IO bpWriter;                      /**< @todo  */
  adios2::Engine engine;                    /**< @todo  */
  adios2::Variable<std::string> identifier; /**< @todo  */
  adios2::Variable<std::string> stage;      /**< @todo  */
  adios2::Variable<std::string> type;       /**< @todo  */
  adios2::Variable<std::string> markdown;   /**< @todo  */
  adios2::Variable<int> result;             /**< @todo  */
  unsigned int outputFile;                  /**< @todo  */

  AdiosEngineManager();
  ~AdiosEngineManager() override;

  void Put(std::string variableName,
           const double& value,const MetaData& m ) override;
  void Put(std::string variableName,
           const long long& value,const MetaData& m) override;
  void Put( std::string variableName,
           const bool& value,const MetaData& m) override;
  void Put( std::string variableName,
           const json& value,const MetaData& m) override;
  void Put( std::string variableName,
           const std::string& value,const MetaData& m) override;

  void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                              const MetaData& m,
                      int onlyOne=-1 ) override
  {}

  void Log(ICommunicator_ptr /** comm **/, const char* package, int stage,
           std::string level, std::string message) override;

  void finalize(ICommunicator_ptr worldComm) override;

  void setFromJson(json& config) override;

  void injectionPointEndedCallBack(ICommunicator_ptr /** comm **/, std::string id,
                                   InjectionPointType type_,
                                   std::string stageId) override;

  void injectionPointStartedCallBack(ICommunicator_ptr /** comm **/,
                                     std::string packageName, std::string id,
                                     InjectionPointType type_,
                                     std::string stageId) override;

  void testStartedCallBack(ICommunicator_ptr /** comm **/, std::string packageName,
                           std::string testName, bool internal) override;

  void testFinishedCallBack(ICommunicator_ptr /** comm **/, bool result_) override;

  void unitTestStartedCallBack(ICommunicator_ptr /** comm **/, std::string packageName,
                               std::string unitTestName) override;
  void unitTestFinishedCallBack(ICommunicator_ptr /** comm **/,
                                IUnitTest* tester) override;

  void dataTypeStartedCallBack( std::string variableName,
                               long long dtype,const MetaData& m) override;
  void dataTypeEndedCallBack(
                             std::string variableName) override;
  json getConfigurationSchema() override;

  std::string print() override;

  Nodes::IRootNode* readFromFile(std::string /**filename**/,
                                 long& idCounter) override;
};

OutputEngineManager* AdiosEngineBuilder();

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif  // ADIOSENGINEMANAGER_H
