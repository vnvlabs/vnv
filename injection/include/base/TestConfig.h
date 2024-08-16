#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <map>
#include <regex>
#include <string>



#include "validate/json-schema.hpp"
using nlohmann::json;

#include "c-interfaces/Wrappers.h"

namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };

class IOutputEngine;

enum class InjectionPointType { Single, Begin, End, Iter, Child_Single, Child_Begin, Child_End, Child_Iter };
namespace InjectionPointTypeUtils {
std::string getType(InjectionPointType type, std::string stageId);
int toC(InjectionPointType type);
}  // namespace InjectionPointTypeUtils

enum class VariableEnum { Double, String, Int, Float, Long };
namespace VariableEnumFactory {
VariableEnum fromString(std::string s);
std::string toString(VariableEnum e);
}  // namespace VariableEnumFactory

typedef std::map<std::string, std::string> MetaData;

class OutputEngineManager;

class VnVParameter {
  void* ptr;
  std::string type;


 public:
  VnVParameter() { ptr = nullptr; }

  VnVParameter(void* obj, std::string type_) {
    type = type_;
    ptr = obj;
  }

  VnVParameter(const VnVParameter& copy) {
    ptr = copy.getRawPtr();
    type = copy.getType();
  }

  void setType(std::string type) { this->type = type; }

  void* getRawPtr() const { return ptr; }

  std::string getType() const { return type; }

  template <typename T> T* getPtr() {
      return (T*)(ptr);
  }

  template <typename T> T& getRef() { return *getPtr<T>(); }
};

class VnVParameterSet : public std::map<std::string, VnVParameter> {
 public:
  template <typename T> T* getPtr(std::string name, bool throwOnError = true) {
    auto it = find(name);
    if (it != end()) {
      return it->second.getPtr<T>();
    }
    if (throwOnError) {
      throw "Parameter Mapping Error";
    }
    return NULL;
  }

  template <typename T> T& getRef(std::string name) {
    auto it = find(name);
    if (it != end()) {
      return it->second.getRef<T>();
    }
    throw "Parameter Mapping Error:";
  }
};

class TestConfig {
 protected:
  VnVParameterSet* parameters;

  std::string testName;
  std::string package;
  nlohmann::json testConfigJson;
  nlohmann::json runConfig;

 public:
  TestConfig(std::string package, std::string name, json& usersConfig);
  json runTemplateName = json::object();

  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;

  /**
   * @brief setName
   * @param name
   */
  void setName(std::string name);

  VnVParameterSet& getParameterMap();

  json& getRunConfig() { return runConfig; }

  void setParameterMap(VnVParameterSet& args);

  /**
   * @brief getName
   * @return
   */
  std::string getName() const;
  std::string getPackage() const;
  /**
   * @brief print out configuration information.
   */
  void print();
};

class VnVCallbackData {
 public:
  VnVCallbackData(VnV_Comm c, VnV::VnVParameterSet& n, VnV::OutputEngineManager* e, VnV::InjectionPointType t,
                  std::string s)
      : comm(c), ntv(n), engine(e), type(t), stageId(s) {}

  VnV_Comm comm;
  VnV::VnVParameterSet& ntv;
  VnV::OutputEngineManager* engine;
  VnV::InjectionPointType type;
  std::string stageId;
};

typedef std::function<void(VnVCallbackData& data)> DataCallback;

}  // namespace VnV

#endif
