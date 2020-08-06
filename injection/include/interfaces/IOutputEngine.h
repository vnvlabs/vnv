#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>

#include "base/CommunicationStore.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/Logging.h"
#include "c-interfaces/PackageName.h"
#include "c-interfaces/Wrappers.h"
#include "interfaces/IUnitTest.h"
#include "interfaces/nodes/Nodes.h"
#include "json-schema.hpp"
/**
 * @brief The IOutputEngine class
 */
using nlohmann::json;

namespace VnV {

enum class InjectionPointType { Single, Begin, End, Iter };
namespace InjectionPointTypeUtils {

std::string getType(InjectionPointType type, std::string stageId);
int toC(InjectionPointType type);
}  // namespace InjectionPointTypeUtils

enum class VariableEnum { Double, String, Int, Float, Long };
namespace VariableEnumFactory {
VariableEnum fromString(std::string s);
std::string toString(VariableEnum e);
}  // namespace VariableEnumFactory

class IOutputEngine {
 protected:
  virtual void dataTypeStartedCallBack(VnV_Comm comm, std::string variableName,
                                       std::string dtype) = 0;
  virtual void dataTypeEndedCallBack(VnV_Comm comm,
                                     std::string variableName) = 0;

 public:
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const double& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const int& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const float& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const long& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const json& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const bool& value) = 0;
  virtual void Put(VnV_Comm comm, std::string variableName,
                   const std::string& value) = 0;
  virtual void Log(VnV_Comm comm, const char* packageName, int stage,
                   std::string level, std::string message) = 0;

  template <typename T>
  void Put(VnV_Comm comm, std::string variableName, T* data) {
    auto it = CommunicationStore::instance().getDataType(typeid(T).name());
    if (it != nullptr) {
      dataTypeStartedCallBack(comm, variableName, typeid(T).name());
      it->setData(data);
      it->Put(comm, this);
      dataTypeEndedCallBack(comm, variableName);
    } else {
      VnV_Warn(VNVPACKAGENAME,
               "Cannot serialize object of type %s -- no DataType "
               "implementatio found",
               typeid(T).name());
    }
  }

  void BeginCustomDataType(VnV_Comm comm, std::string variableName) {
    dataTypeStartedCallBack(comm, variableName, "__custom__");
  }

  void EndCustomDataType(VnV_Comm comm, std::string variableName) {
    dataTypeEndedCallBack(comm, variableName);
  }

  void Put(VnV_Comm comm, std::string variableName, std::string serializer,
           std::string inputType, void* object);

  virtual ~IOutputEngine() = default;
};

class IInternalOutputEngine : public IOutputEngine {
 public:
  virtual void setFromJson(json& configuration) = 0;
  virtual json getConfigurationSchema() = 0;
  virtual void injectionPointStartedCallBack(VnV_Comm comm,
                                             std::string packageName,
                                             std::string id,
                                             InjectionPointType type,
                                             std::string stageId) = 0;
  virtual void injectionPointEndedCallBack(VnV_Comm comm, std::string id,
                                           InjectionPointType type,
                                           std::string stageId) = 0;
  virtual void testStartedCallBack(VnV_Comm comm, std::string packageName,
                                   std::string testName, bool internal) = 0;
  virtual void testFinishedCallBack(VnV_Comm comm, bool result_) = 0;
  virtual void unitTestStartedCallBack(VnV_Comm comm, std::string packageName,
                                       std::string unitTestName) = 0;
  virtual void unitTestFinishedCallBack(VnV_Comm comm, IUnitTest* tester) = 0;

  virtual Nodes::IRootNode* readFromFile(std::string file, long& idCounter) = 0;
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
  IOutputEngine* getOutputEngine();
  virtual ~OutputEngineManager() = default;
};

typedef OutputEngineManager* engine_register_ptr();

void registerEngine(std::string name, VnV::engine_register_ptr r);

}  // namespace VnV

#define INJECTION_ENGINE(PNAME, name)                               \
  namespace VnV {                                                   \
  namespace PNAME {                                                 \
  namespace Engines {                                               \
  OutputEngineManager* declare_##name();                            \
  void register_##name() { registerEngine(#name, declare_##name); } \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  VnV::OutputEngineManager* VnV::PNAME::Engines::declare_##name()

#define DECLAREENGINE(PNAME, name) \
  namespace VnV {                  \
  namespace PNAME {                \
  namespace Engines {              \
  void register_##name();          \
  }                                \
  }                                \
  }
#define REGISTERENGINE(PNAME, name) VnV::PNAME::Engines::register_##name();

#endif  // IOUTPUTENGINE_H
