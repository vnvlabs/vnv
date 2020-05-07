#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>
#include "json-schema.hpp"
#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/Nodes.h"
#include "python/PythonInterface.h"

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
}

class IOutputEngine {
public:
  virtual void Put(VnV_Comm comm, std::string variableName, const double& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName, const int& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName, const float& value) =0;
  virtual void Put(VnV_Comm comm,std::string variableName, const long& value) = 0;
  virtual void Put(VnV_Comm comm,std::string variableName, const json& value) = 0;
  virtual void Put(VnV_Comm comm,std::string variableName, const bool& value) = 0 ;
  virtual void Put(VnV_Comm comm,std::string variableName, const std::string& value) = 0;
  virtual void Log(VnV_Comm comm,const char * packageName, int stage, std::string level, std::string message) = 0;
  void Put(VnV_Comm comm,std::string variableName, std::string serializer, std::string inputType, void* object);
  virtual ~IOutputEngine() = default;

};

class IInternalOutputEngine : public IOutputEngine {
public:
    virtual void setFromJson(json& configuration) = 0;
    virtual json getConfigurationSchema() = 0;
    virtual void injectionPointStartedCallBack(VnV_Comm comm, std::string id, InjectionPointType type, std::string stageId) = 0;
    virtual void injectionPointEndedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageId) = 0;
    virtual void documentationEndedCallBack(VnV_Comm comm,std::string pname, std::string id) = 0;
    virtual void documentationStartedCallBack(VnV_Comm comm, std::string pname, std::string id) = 0;
    virtual void testStartedCallBack(VnV_Comm comm,std::string testName) = 0;
    virtual void testFinishedCallBack(VnV_Comm comm,bool result_) = 0;
    virtual void unitTestStartedCallBack(VnV_Comm comm, std::string unitTestName) = 0;
    virtual void unitTestFinishedCallBack(VnV_Comm comm,std::map<std::string,bool> &results) = 0;

    virtual Nodes::IRootNode*  readFromFile(std::string file) = 0;
    virtual std::string print() = 0;
    virtual void finalize() = 0;
    virtual ~IInternalOutputEngine() = default;
};

/**
 * @brief The OutputEngineManager class
 */
class OutputEngineManager : public IInternalOutputEngine {
public:

  void set(json& configuration);
  void document(VnV_Comm comm, std::string pname, std::string id, std::map<std::string,std::pair<std::string,void*>> &map);
  IOutputEngine* getOutputEngine();
  virtual ~OutputEngineManager() = default;
};

typedef OutputEngineManager* engine_register_ptr();

void registerEngine(std::string name, VnV::engine_register_ptr r);

}



#endif // IOUTPUTENGINE_H
