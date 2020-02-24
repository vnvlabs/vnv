#ifndef VV_ADIOS_HEADER
#define VV_ADIOS_HEADER

/**
 * @file vv-utils.h
 */
#include <string>

#include "adios2.h"
#include "interfaces/IOutputEngine.h"
/**
 * VnV Namespace.
 */
namespace VnV {
/**
 * @brief The AdiosEngine class
 */

	
class AdiosEngine : public IOutputEngine {
 public:
  adios2::IO& writer;
  adios2::Engine& engine;

  AdiosEngine(adios2::Engine& _engine, adios2::IO& _io);


  /**
   * @brief Log
   * @param log
   */
  void Log(const char * package, int stage, std::string level, std::string message) override;

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, double& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, int& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, float& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, long& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  void Put(std::string variableName, std::string& value);

  /**
   * @brief Define IO Variable
   * @param name
   */
  void Define(VariableEnum type, std::string name);

};

class AdiosWrapper : public OutputEngineManager {
 private:
  adios2::ADIOS* adios; /**< @todo  */

  adios2::IO bpWriter;                      /**< @todo  */
  adios2::Engine engine;                    /**< @todo  */
  adios2::Variable<std::string> identifier; /**< @todo  */
  adios2::Variable<std::string> stage;              /**< @todo  */
  adios2::Variable<std::string> type;       /**< @todo  */
  adios2::Variable<std::string> markdown;   /**< @todo  */
  adios2::Variable<int> result;             /**< @todo  */
  unsigned int outputFile;                  /**< @todo  */

  AdiosEngine* adiosEngine; /**< @todo  */

 public:
  /**
   * @brief AdiosWrapper
   */
  AdiosWrapper();


  void getIndent(int stage);

  /**
   * @brief finalize
   */
  void finalize();

  /**
   * @brief set
   * @param config
   */
  void set(json& config);

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageId) override;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void injectionPointStartedCallBack(std::string id, InjectionPointType type, std::string stageId) override;

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void testStartedCallBack(std::string testName) override;

  /**
   * @brief stopTest
   * @param result_
   */
  void testFinishedCallBack(bool result_) override;


  void unitTestStartedCallBack(std::string unitTestName) override;

  void unitTestFinishedCallBack(std::map<std::string,bool> &results) override;

  /**
   * @brief get the configuration schema for the adios engine. 
   */
  json getConfigurationSchema() override;

  /**
   * @brief getOutputEngine
   * @return
   */
  IOutputEngine* getOutputEngine();
};

}  // namespace VnV

#endif
