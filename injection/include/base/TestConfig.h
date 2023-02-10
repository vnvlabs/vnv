#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <map>
#include <regex>
#include <string>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/stores/TransformStore.h"
#include "c-interfaces/Wrappers.h"
#include "common-interfaces/Communication.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };

class IOutputEngine;

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

typedef std::map<std::string, std::string> MetaData;

class OutputEngineManager;

class VnVParameter {
  void* ptr;
  std::string type;

  std::shared_ptr<Transformer> trans = nullptr;
  std::string transType = "";

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

  template <typename T> T* getPtr(std::string requestedType, bool throwOnError = true) {
    StringUtils::squash(type);

    if (requestedType.empty()) {  // Type checking turned off.
      return (T*)(ptr);
    }

    if (trans == nullptr || type.compare(transType) != 0) {
      trans = TransformStore::instance().getTransformer(type, requestedType);
      transType = requestedType;
    }

    if (trans != nullptr && type.compare(transType) == 0) {
      return static_cast<T*>(trans->Transform(ptr));
    }

    if (throwOnError) {
      HTHROW INJECTION_EXCEPTION("Bad Transform Requested -- Cannot transform from %s -> %s", type.c_str(),
                                 requestedType.c_str());
    }
    return NULL;
  }

  template <typename T> T& getRef(std::string type) { return *getPtr<T>(type, true); }
};

class VnVParameterSet : public std::map<std::string, VnVParameter> {
 public:
  template <typename T> T* getPtr(std::string name, std::string type, bool throwOnError = true) {
    StringUtils::squash(type);

    auto it = find(name);
    if (it != end()) {
      return it->second.getPtr<T>(type, throwOnError);
    }
    if (throwOnError) {
      HTHROW INJECTION_EXCEPTION("Parameter Mapping Error. No parameter called %s exists", name.c_str());
    }
    return NULL;
  }

  template <typename T> T& getRef(std::string name, std::string type) {
    StringUtils::squash(type);
    auto it = find(name);
    if (it != end()) {
      return it->second.getRef<T>(type);
    }
    HTHROW INJECTION_EXCEPTION("Parameter Mapping Error: No parameter named %s exists. ", name.c_str());
  }
};

class TestConfig {
 protected:
  std::map<std::string, std::shared_ptr<Transformer>> transformers;
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
