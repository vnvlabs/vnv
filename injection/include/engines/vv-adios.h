#ifndef VV_ADIOS_HEADER
#define VV_ADIOS_HEADER

/**
 * @file vv-utils.h
 */
#include <string>

#include "adios2.h"
#include "vv-output.h"
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
   * @brief DefineDouble
   * @param name
   */
  void DefineDouble(std::string name);

  /**
   * @brief DefineFloat
   * @param name
   */
  void DefineFloat(std::string name);

  /**
   * @brief DefineInt
   * @param name
   */
  void DefineInt(std::string name);

  /**
   * @brief DefineLong
   * @param name
   */
  void DefineLong(std::string name);

  /**
   * @brief DefineString
   * @param name
   */
  void DefineString(std::string name);
};

class AdiosWrapper : public OutputEngineManager {
 private:
  adios2::ADIOS* adios; /**< @todo  */

  adios2::IO bpWriter;                      /**< @todo  */
  adios2::Engine engine;                    /**< @todo  */
  adios2::Variable<std::string> identifier; /**< @todo  */
  adios2::Variable<int> stage;              /**< @todo  */
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
  void endInjectionPoint(std::string id, int stageVal);

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  void startInjectionPoint(std::string id, int stageVal);

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  void startTest(std::string testName, int testStageVal);

  /**
   * @brief stopTest
   * @param result_
   */
  void stopTest(bool result_);

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
