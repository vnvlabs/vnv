#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>
#include "json-schema.hpp"
#include "c-interfaces/PackageName.h"
//#include "base/InjectionPoint.h"
/**
 * @brief The IOutputEngine class
 */
using nlohmann::json;

namespace VnV {

enum class InjectionPointType {Single, Begin, End, Iter};
namespace InjectionPointTypeUtils {
   std::string getType(InjectionPointType type, std::string stageId);
}

enum class VariableEnum {Double, String, Int, Float, Long};
namespace VariableEnumFactory {
    VariableEnum fromString(std::string s);
    std::string toString(VariableEnum e);
};

class IOutputEngine {

public:

   std::string getIndent(int stage);

   /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm, std::string variableName, const double& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm, std::string variableName, const int& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm, std::string variableName, const float& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm,std::string variableName, const long& value);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm,std::string variableName, const json& value);

   /**
   * @brief Put a variable using a registered serializer. Serializers are objects that allow
   * converting a certain object to and from strings.
   * @param variableName
   * @param serializer
   * @param inputType
   * @param object
   */
  void Put(VnV_Comm comm,std::string variableName, std::string serializer, std::string inputType, void* object);

  /**
   * @brief Put
   * @param variableName
   * @param value
   */
  virtual void Put(VnV_Comm comm,std::string variableName, const std::string& value);

  /**
   * @brief Log a message to the engine logs.
   * @param packageName
   * @param stage
   * @param level
   * @param message
   */
  virtual void Log(VnV_Comm comm,const char * packageName, int stage, std::string level, std::string message);


    /**
   * @brief Define IO variables that will be written.
   * @param type
   * @param name
   */
  virtual void Define(VariableEnum type, std::string name) = 0;

  /**
   * @brief ~IOutputEngine
   */
  virtual ~IOutputEngine();
};




/**
 * @brief The OutputEngineManager class
 */
class OutputEngineManager {
   friend class RunTime;
   friend class UnitTestStore;
   friend class InjectionPoint;
   friend class OutputEngineStore;
   friend class Logger;
   friend class ITest;

private:
  /**
   * @brief _set
   * @param configuration
   */
  void _set(json& configuration);
  /**
   * @brief set
   * @param configuration
   */
  virtual void set(json& configuration) = 0;

  /**
   * @brief getConfigurationSchema
   * @return
   */
  virtual json getConfigurationSchema();

  virtual void print();

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void injectionPointEndedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageId) = 0;

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void injectionPointStartedCallBack(VnV_Comm comm, std::string id, InjectionPointType type, std::string stageId) = 0;

  /**
   * @brief endInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void documentationEndedCallBack(VnV_Comm comm,std::string pname, std::string id);

  /**
   * @brief startInjectionPoint
   * @param id
   * @param stageVal
   */
  virtual void documentationStartedCallBack(VnV_Comm comm, std::string pname, std::string id);

  /**
   * @brief startTest
   * @param testName
   * @param testStageVal
   */
  virtual void testStartedCallBack(VnV_Comm comm,std::string testName) = 0;

  /**
   * @brief stopTest
   * @param result_
   */
  virtual void testFinishedCallBack(VnV_Comm comm,bool result_) = 0;


  virtual void unitTestStartedCallBack(VnV_Comm comm, std::string unitTestName) = 0;

  virtual void unitTestFinishedCallBack(VnV_Comm comm,std::map<std::string,bool> &results) = 0;

  /**
   * @brief finalize
   */
  virtual void finalize() = 0;

  virtual void document(VnV_Comm comm, std::string pname, std::string id, std::map<std::string,std::pair<std::string,void*>> &map);

  virtual void Log(VnV_Comm comm,const char * packageName, int stage, std::string level, std::string message);
  virtual IOutputEngine* getOutputEngine() = 0;

public:

  virtual void Put(VnV_Comm comm, std::string variableName, const double& value);
  virtual void Put(VnV_Comm comm, std::string variableName, const int& value);
  virtual void Put(VnV_Comm comm, std::string variableName, const float& value);
  virtual void Put(VnV_Comm comm,std::string variableName, const long& value);
  virtual void Put(VnV_Comm comm,std::string variableName, const json& value);
  virtual void Put(VnV_Comm comm,std::string variableName, std::string serializer, std::string inputType, void* object);
  virtual void Put(VnV_Comm comm,std::string variableName, const std::string& value);
  virtual ~OutputEngineManager();
};

typedef OutputEngineManager* engine_register_ptr();

void registerEngine(std::string name, VnV::engine_register_ptr r);

}



#endif // IOUTPUTENGINE_H
