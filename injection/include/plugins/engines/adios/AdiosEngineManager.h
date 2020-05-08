#ifndef ADIOSENGINEMANAGER_H
#define ADIOSENGINEMANAGER_H

#include "base/exceptions.h"
#include <sstream>
#include <string>
#include <map>

#include "adios2.h"
#include "json-schema.hpp"
#include "interfaces/IOutputEngine.h"

using nlohmann::json;


namespace VnV {
namespace Engines {

class AdiosEngineManager : public OutputEngineManager {
public:
    adios2::ADIOS* adios; /**< @todo  */
    adios2::IO bpWriter;                      /**< @todo  */
    adios2::Engine engine;                    /**< @todo  */
    adios2::Variable<std::string> identifier; /**< @todo  */
    adios2::Variable<std::string> stage;              /**< @todo  */
    adios2::Variable<std::string> type;       /**< @todo  */
    adios2::Variable<std::string> markdown;   /**< @todo  */
    adios2::Variable<int> result;             /**< @todo  */
    unsigned int outputFile;                  /**< @todo  */

    AdiosEngineManager();
    ~AdiosEngineManager() override ;

    void Put(VnV_Comm /** comm **/, std::string variableName, const double& value)override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const int& value)override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const float& value)override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const long& value)override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const bool& value)override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const json& value) override;
    void Put(VnV_Comm /** comm **/,std::string variableName, const std::string& value)override;

    void Log(VnV_Comm /** comm **/,const char* package, int stage, std::string level, std::string message)override;

    void finalize()override;

    void setFromJson(json& config)override;

    void injectionPointEndedCallBack(VnV_Comm /** comm **/, std::string id, InjectionPointType type_, std::string stageId)override;

    void injectionPointStartedCallBack(VnV_Comm /** comm **/, std::string id, InjectionPointType type_, std::string stageId)override;

    void testStartedCallBack(VnV_Comm /** comm **/, std::string testName)override;

    void testFinishedCallBack(VnV_Comm /** comm **/, bool result_)override;

    void unitTestStartedCallBack(VnV_Comm /** comm **/, std::string unitTestName)override;
    void unitTestFinishedCallBack(VnV_Comm /** comm **/,std::map<std::string,bool> &results)override;

    json getConfigurationSchema()override;

    std::string print() override;

    Nodes::IRootNode* readFromFile(std::string /**filename**/ )override;
};

OutputEngineManager* AdiosEngineBuilder();

}
}
#endif // ADIOSENGINEMANAGER_H
